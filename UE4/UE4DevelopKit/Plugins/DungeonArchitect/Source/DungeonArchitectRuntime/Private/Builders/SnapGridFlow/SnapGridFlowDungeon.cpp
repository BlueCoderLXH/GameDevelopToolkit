//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SnapGridFlow/SnapGridFlowDungeon.h"

#include "Builders/SnapGridFlow/SnapGridFlowAsset.h"
#include "Core/Dungeon.h"
#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph3D.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Utils/GridFlowAbstractGraphVisualization.h"
#include "Frameworks/Flow/Domains/Snap/SnapFlowAbstractGraphSupport.h"
#include "Frameworks/Flow/Domains/Snap/SnapFlowDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecGraphScript.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "Frameworks/Flow/FlowProcessor.h"
#include "Frameworks/Snap/Lib/Theming/SnapTheme.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowGraphSerialization.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowLibrary.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleBounds.h"
#include "Frameworks/ThemeEngine/DungeonThemeEngine.h"
#include "Frameworks/ThemeEngine/Markers/PlaceableMarker.h"

#include "EngineUtils.h"

class UGridFlowAbstractGraph3D;
DEFINE_LOG_CATEGORY_STATIC(SnapGridDungeonBuilderLog, Log, All);

///////////////////////////// USnapGridFlowBuilder /////////////////////////////
void USnapGridFlowBuilder::BuildNonThemedDungeonImpl(UWorld* World, TSharedPtr<FDungeonSceneProvider> SceneProvider) {
    if (!Dungeon)
    {
        UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Invalid 'Dungeon' object"));
        return;
    }
    
    SnapGridModel = Cast<USnapGridFlowModel>(model);
    SnapGridConfig = Cast<USnapGridFlowConfig>(config);

    if (!World || !SnapGridConfig.IsValid() || !SnapGridModel.IsValid()) {
        UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Invalid reference passed to the snap builder"));
        return;
    }

    USnapGridFlowAsset* FlowAsset = SnapGridConfig->FlowGraph.LoadSynchronous();
    if (!FlowAsset) {
        UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Dungeon Flow asset not specified"));
        return;
    }

    USnapGridFlowModuleDatabase* ModuleDatabase = SnapGridConfig->ModuleDatabase.LoadSynchronous();
    if (!ModuleDatabase) {
        UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Module Database asset is not specified"));
        return;
    }
    
    if (LevelStreamHandler.IsValid()) {
        LevelStreamHandler->ClearStreamingLevels();
        LevelStreamHandler.Reset();
    }
    
    SnapGridModel->Reset();
    PropSockets.Reset();

    UDungeonLevelStreamingModel* LevelStreamModel = Dungeon ? Dungeon->LevelStreamingModel : nullptr;
    LevelStreamHandler = MakeShareable(new FSnapGridFlowStreamingChunkHandler(GetWorld(), SnapGridModel.Get(), LevelStreamModel));
    LevelStreamHandler->ClearStreamingLevels();
    LevelStreamHandler->ChunkLoadedEvent.BindUObject(this, &USnapGridFlowBuilder::HandleChunkLoaded);
    LevelStreamHandler->ChunkFullyLoadedEvent.BindUObject(this, &USnapGridFlowBuilder::HandleChunkLoadedAndVisible);
    
    ExecuteFlowGraph();
    
    TArray<SnapLib::FModuleNodePtr> ModuleGraphNodes;
    GenerateModuleNodeGraph(ModuleGraphNodes);
    FSnapGridFlowGraphSerializer::Serialize(ModuleGraphNodes, SnapGridModel->ModuleInstances, SnapGridModel->Connections);

    TSet<FGuid> SpawnRoomNodes;
    if (SnapGridModel->AbstractGraph) {
        for (UFlowAbstractNode* Node : SnapGridModel->AbstractGraph->GraphNodes) {
            if (FFlowAbstractGraphUtils::ContainsItem(Node->NodeItems, EFlowGraphItemType::Entrance)) {
                SpawnRoomNodes.Add(Node->NodeId);
            }
        } 
    }
    
    FSnapStreaming::GenerateLevelStreamingModel(World, ModuleGraphNodes, Dungeon->LevelStreamingModel, USnapStreamingChunk::StaticClass(),
        [this, &SpawnRoomNodes](UDungeonStreamingChunk* InChunk, SnapLib::FModuleNodePtr Node)  {
            USnapStreamingChunk* Chunk = Cast<USnapStreamingChunk>(InChunk);
            if (Chunk) {
                Chunk->ModuleTransform = Node->WorldTransform;
                if (SpawnRoomNodes.Contains(Node->ModuleInstanceId)) {
                    Chunk->bSpawnRoomChunk = true;
                }
                
                if (LevelStreamHandler.IsValid()) {
                    LevelStreamHandler->RegisterEvents(Chunk);
                }
            }
        });

    // Create the debug draw actor
    if (Dungeon) {
        if (Dungeon->bDrawDebugData) {
            CreateDebugVisualizations(Dungeon->Uid);
        }
        else {
            DestroyDebugVisualizations(Dungeon->Uid);
        }
    }
}

bool USnapGridFlowBuilder::GenerateModuleNodeGraph(TArray<SnapLib::FModuleNodePtr>& OutNodes) const {
    if (!SnapGridConfig.IsValid()) {
        return false;
    }
    
    USnapGridFlowModuleDatabase* ModuleDatabase = SnapGridConfig->ModuleDatabase.LoadSynchronous();
    if (!ModuleDatabase || !ModuleDatabase->ModuleBoundsAsset) {
        return false;
    }
    
    if (!SnapGridModel->AbstractGraph) {
        return false;
    }
    
    FSnapGridFlowGraphLatticeGenSettings GenSettings;
    GenSettings.Seed = SnapGridConfig->Seed;
    GenSettings.ChunkSize = ModuleDatabase->ModuleBoundsAsset->ChunkSize;
    GenSettings.ModulesWithMinimumDoorsProbability = SnapGridConfig->PreferModulesWithMinimumDoors;
    GenSettings.BaseTransform = Dungeon ? Dungeon->GetActorTransform() : FTransform::Identity;
    const SnapLib::IModuleDatabasePtr ModDB = MakeShareable(new FSnapGridFlowModuleDatabaseImpl(ModuleDatabase));
    const FSnapGridFlowGraphLatticeGenerator GraphGenerator(ModDB, GenSettings);
    
    return GraphGenerator.Generate(SnapGridModel->AbstractGraph, OutNodes);
}

bool USnapGridFlowBuilder::ExecuteFlowGraph() {
    SnapGridModel->AbstractGraph = nullptr;

    USnapGridFlowAsset* SnapGridFlowAsset = SnapGridConfig->FlowGraph.LoadSynchronous();
    USnapGridFlowModuleDatabase* ModuleDatabase = SnapGridConfig->ModuleDatabase.LoadSynchronous();

    if (!SnapGridFlowAsset) {
        UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Missing Grid Flow graph"));
        return false;
    }

    if (!SnapGridFlowAsset->ExecScript) {
        UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Invalid Grid Flow graph state. Please resave in editor"));
        return false;
    }

    FFlowProcessor FlowProcessor;

    // Register the domains
    {
        FSnapGridFlowProcessDomainExtender Extender(ModuleDatabase);
        Extender.ExtendDomains(FlowProcessor);
    }
    
    const int32 MAX_RETRIES = FMath::Max(1, SnapGridConfig->NumLayoutBuildRetries);
    int32 NumTries = 0;
    int32 NumTimeouts = 0;
    FFlowProcessorResult Result;
    while (NumTries < MAX_RETRIES) {
        FFlowProcessorSettings FlowProcessorSettings;
        FlowProcessorSettings.AttributeList = AttributeList;
        FlowProcessorSettings.SerializedAttributeList = SnapGridConfig->ParameterOverrides;
        Result = FlowProcessor.Process(SnapGridFlowAsset->ExecScript, random, FlowProcessorSettings);
        NumTries++;
        if (Result.ExecResult == EFlowTaskExecutionResult::Success) {
            break;
        }

        if (Result.ExecResult == EFlowTaskExecutionResult::FailHalt) {
            bool bHalt = true;

            if (Result.FailReason == EFlowTaskExecutionFailureReason::Timeout) {
                NumTimeouts++;
                if (NumTimeouts <= SnapGridConfig->NumTimeoutsRetriesAllowed) {
                    // Continue despite the timeout
                    bHalt = false;
                }
            }

            if (bHalt) {
                break;
            }
        }
    }

    if (Result.ExecResult != EFlowTaskExecutionResult::Success) {
        UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Failed to generate grid flow graph"));
        return false;
    }

    if (!SnapGridFlowAsset->ExecScript->ResultNode) {
        UE_LOG(SnapGridDungeonBuilderLog, Error,
               TEXT(
                   "Cannot find result node in the grid flow exec graph. Please resave the grid flow asset in the editor"
               ));
        return false;
    }

    const FGuid ResultNodeId = SnapGridFlowAsset->ExecScript->ResultNode->NodeId;
    if (FlowProcessor.GetNodeExecStage(ResultNodeId) != EFlowTaskExecutionStage::Executed) {
        UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Grid Flow Graph execution failed"));
        return false;
    }

    FFlowExecutionOutput ResultNodeState;
    FlowProcessor.GetNodeState(ResultNodeId, ResultNodeState);
    if (ResultNodeState.ExecutionResult != EFlowTaskExecutionResult::Success) {
        UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Grid Flow Result node execution did not succeed"));
        return false;
    }

    // Save a copy in the model
    UGridFlowAbstractGraph3D* TemplateGraph = ResultNodeState.State->GetState<UGridFlowAbstractGraph3D>(UFlowAbstractGraphBase::StateTypeID);
    SnapGridModel->AbstractGraph = NewObject<UGridFlowAbstractGraph3D>(SnapGridModel.Get(), "AbstractGraph", RF_NoFlags, TemplateGraph);
    return true;
}

void USnapGridFlowBuilder::DestroyNonThemedDungeonImpl(UWorld* World) {
    UDungeonBuilder::DestroyNonThemedDungeonImpl(World);
    SnapGridModel = Cast<USnapGridFlowModel>(model);
    SnapGridConfig = Cast<USnapGridFlowConfig>(config);


    if (LevelStreamHandler.IsValid()) {
        LevelStreamHandler->ClearStreamingLevels();
        LevelStreamHandler.Reset();
    }
    
    SnapGridModel->Reset();
    
    // Create the debug draw actor of specified
    if (Dungeon) {
        DestroyDebugVisualizations(Dungeon->Uid);
    }
}

void USnapGridFlowBuilder::CreateDebugVisualizations(const FGuid& DungeonID) const {
    DestroyDebugVisualizations(DungeonID);

    if (!SnapGridModel.IsValid() || !SnapGridConfig.IsValid()) {
        return;
    }
    USnapGridFlowModuleDatabase* ModuleDatabase = SnapGridConfig->ModuleDatabase.LoadSynchronous();
    if (SnapGridModel->AbstractGraph && ModuleDatabase && ModuleDatabase->ModuleBoundsAsset) {
        const FVector ModuleSize = ModuleDatabase->ModuleBoundsAsset->ChunkSize;
    
        UWorld* World = GetWorld();
        AGridFlowAbstractGraphVisualizer* Visualizer = World->SpawnActor<AGridFlowAbstractGraphVisualizer>();
        Visualizer->DungeonID = DungeonID;
        Visualizer->SetAutoAlignToLevelViewport(true);
        if (SnapGridModel.IsValid() && SnapGridModel->AbstractGraph) {
            FGFAbstractGraphVisualizerSettings Settings;
            const float ModuleWidth = FMath::Min(ModuleSize.X, ModuleSize.Y);
            Settings.NodeRadius = ModuleWidth * 0.05;
            Settings.LinkThickness = Settings.NodeRadius * 0.2f;
            Settings.LinkRefThickness = Settings.LinkThickness * 0.5f;
            Settings.NodeSeparationDistance = ModuleSize;
            Settings.bRenderNodeOnCellCenter = true;
            Visualizer->Generate(SnapGridModel->AbstractGraph, Settings);
        }
    }
}

void USnapGridFlowBuilder::DestroyDebugVisualizations(const FGuid& DungeonID) const {
    UWorld* World = GetWorld();
    for (TActorIterator<AGridFlowAbstractGraphVisualizer> It(World); It; ++It) {
        AGridFlowAbstractGraphVisualizer* Visualizer = *It;
        if (Visualizer && Visualizer->DungeonID == DungeonID) {
            Visualizer->Destroy();
        }
    }
}

void USnapGridFlowBuilder::SpawnItem(UFlowGraphItem* ItemInfo, APlaceableMarkerActor* InMarkerActor, USnapStreamingChunk* InChunk) const {
    if (!SnapGridConfig.IsValid() || !ItemInfo) return;

    const FString MarkerName = ItemInfo->MarkerName.TrimStartAndEnd();
    
    TArray<FPropSocket> MarkersToEmit;
    FPropSocket& Marker = MarkersToEmit.AddDefaulted_GetRef();
    Marker.Id = 0;
    Marker.SocketType = MarkerName;
    Marker.Transform = InMarkerActor->GetActorTransform();

    TSharedPtr<FSnapThemeSceneProvider> SceneProvider = MakeShareable(new FSnapThemeSceneProvider(GetWorld()));
    SceneProvider->SetLevelOverride(InMarkerActor->GetLevel());
    
    FDungeonThemeEngineSettings ThemeEngineSettings;
    ThemeEngineSettings.Themes = { SnapGridConfig->ItemTheme.LoadSynchronous() };
    ThemeEngineSettings.SceneProvider = SceneProvider;

    const FDungeonThemeEngineEventHandlers ThemeEventHandlers;
    
    // Invoke the Theme Engine
    FDungeonThemeEngine::Apply(MarkersToEmit, random, ThemeEngineSettings, ThemeEventHandlers);

    // Register the spawned actors so they can be cleaned up when the chunk is destroyed
    for (TWeakObjectPtr<AActor> SpawnedActor : SceneProvider->GetSpawnedActors()) {
        if (SpawnedActor.IsValid()) {
            InChunk->RegisterManagedActor(SpawnedActor.Get());

            // If it contains a item metadata component, then assign the item
            if (UDungeonFlowItemMetadataComponent* ItemComponent = SpawnedActor->FindComponentByClass<UDungeonFlowItemMetadataComponent>()) {
                ItemComponent->SetFlowItem(ItemInfo);
            }
        }
    }
}

void USnapGridFlowBuilder::HandleChunkLoaded(USnapStreamingChunk* InChunk) {
    if (!InChunk) return;
    
    UWorld* World = GetWorld();
    if (World && !World->IsGameWorld()) {
        // We are building this in the editor.
        // Disable the module bounds rendering

        ULevel* Level = InChunk->GetLoadedLevel();
        if (Level) {
            for (AActor* Actor : Level->Actors) {
                if (ASnapGridFlowModuleBoundsActor* BoundsActor = Cast<ASnapGridFlowModuleBoundsActor>(Actor)) {
                    if (BoundsActor->BoundsComponent) {
                        BoundsActor->BoundsComponent->bRenderBounds = false;
                    }
                }
            }
        }
    }
}

void USnapGridFlowBuilder::HandleChunkLoadedAndVisible(USnapStreamingChunk* InChunk) {
    if (!InChunk) return;
    
    // Spawn items
    if (!SnapGridModel.IsValid() || !SnapGridModel->AbstractGraph) return;

    // Populate the requested marker list from the abstract node 
    TArray<UFlowGraphItem*> Items;
    {
        // Grab the node from the abstract graph. The chunk id will be the same as the abstract node id
        UFlowAbstractNode* AbstractNode = SnapGridModel->AbstractGraph->GetNode(InChunk->ID);

        // Check if abstract node has any items to spawn
        if (AbstractNode) {
            for (UFlowGraphItem* Item : AbstractNode->NodeItems) {
                if (Item) {
                    Items.Add(Item);
                }
            }
        }
    }

    TArray<APlaceableMarkerActor*> PlaceableMarkers;
    for (AActor* Actor : InChunk->GetLoadedLevel()->Actors) {
        if (APlaceableMarkerActor* PlaceableMarker = Cast<APlaceableMarkerActor>(Actor)) {
            PlaceableMarkers.Add(PlaceableMarker);
        }
    }

    TMap<APlaceableMarkerActor*, FString> MarkersToEmit;
    const int32 MarkerRandomSeed = HashCombine(SnapGridConfig->Seed, GetTypeHash(InChunk->Bounds.GetCenter()));
    const FRandomStream RandomMarkerSelection(MarkerRandomSeed);
    FMathUtils::Shuffle(PlaceableMarkers, RandomMarkerSelection);
    for (UFlowGraphItem* ItemInfo : Items) {
        FString Marker = ItemInfo->MarkerName.TrimStartAndEnd();
        if (Marker.Len() == 0) continue;
        
        APlaceableMarkerActor* MarkerActor = nullptr;
        for (APlaceableMarkerActor* PlaceableMarker : PlaceableMarkers) {
            if (PlaceableMarker->GetAllowedMarkerNames().Contains(Marker)) {
                MarkerActor = PlaceableMarker;
                break;
            }
        }

        if (MarkerActor) {
            PlaceableMarkers.Remove(MarkerActor);
            SpawnItem(ItemInfo, MarkerActor, InChunk);
        } else {
            UE_LOG(SnapGridDungeonBuilderLog, Error, TEXT("Cannot spawn marker \"%s\" in module chunk: %s"), *Marker, *InChunk->GetLoadedLevel()->GetFullName());
        }
    }
}

void USnapGridFlowBuilder::DrawDebugData(UWorld* InWorld, bool bPersistent, float LifeTime) {
    /*
    if (!SnapGridModel.IsValid() || !SnapGridConfig.IsValid()) {
        return;
    }

    if (!SnapGridModel->AbstractGraph) {
        return;
    }
    
    if (!SnapGridConfig->ModuleDatabase || !SnapGridConfig->ModuleDatabase->ModuleBoundsAsset) {
        return;
    }

    FVector BaseOffset = Dungeon ? Dungeon->GetActorLocation() : FVector::ZeroVector;
    const FColor BoxColor = FColor(128, 0, 0);
    const FVector ModuleSize = SnapGridConfig->ModuleDatabase->ModuleBoundsAsset->ChunkSize;
    const FIntVector LayoutGridSize = SnapGridModel->AbstractGraph->GridSize;
    for (int z = 0; z < LayoutGridSize.Z; z++) {
        for (int y = 0; y < LayoutGridSize.Y; y++) {
            for (int x = 0; x < LayoutGridSize.X; x++) {
                const FVector BoxMin = BaseOffset + FVector(x, y, z) * ModuleSize;
                const FVector BoxExtent = ModuleSize * 0.5f;
                const FVector BoxCenter = BoxMin + BoxExtent;
                DrawDebugBox(InWorld, BoxCenter, BoxExtent, BoxColor, bPersistent, LifeTime);
            }
        }
    }
    */
}

bool USnapGridFlowBuilder::SupportsProperty(const FName& PropertyName) const {
    if (PropertyName == "Themes") return false;
    if (PropertyName == "MarkerEmitters") return false;
    //if (PropertyName == "EventListeners") return false;

    return  true;
}

bool USnapGridFlowBuilder::CanBuildDungeon(FString& OutMessage) {
    ADungeon* OuterDungeon = Cast<ADungeon>(GetOuter());
    if (OuterDungeon) {
        SnapGridConfig = Cast<USnapGridFlowConfig>(OuterDungeon->GetConfig());

        if (!SnapGridConfig.IsValid()) {
            OutMessage = "Dungeon not initialized correctly";
            return false;
        }

        USnapGridFlowAsset* FlowAsset = SnapGridConfig->FlowGraph.LoadSynchronous();
        if (!FlowAsset) {
            OutMessage = "Dungeon Flow asset not assigned";
            return false;
        }

        UDungeonThemeAsset* ItemThemeAsset = SnapGridConfig->ItemTheme.LoadSynchronous();
        if (!ItemThemeAsset) {
            OutMessage = "Item Theme asset not assigned";
            return false;
        }

        USnapGridFlowModuleDatabase* ModuleDatabase = SnapGridConfig->ModuleDatabase.LoadSynchronous();
        if (!ModuleDatabase) {
            OutMessage = "Module Database asset not assigned";
            return false;
        }
    }
    else {
        OutMessage = "Dungeon not initialized correctly";
        return false;
    }

    return true;
}

TSubclassOf<UDungeonModel> USnapGridFlowBuilder::GetModelClass() {
    return USnapGridFlowModel::StaticClass();
}

TSubclassOf<UDungeonConfig> USnapGridFlowBuilder::GetConfigClass() {
    return USnapGridFlowConfig::StaticClass();
}

TSubclassOf<UDungeonToolData> USnapGridFlowBuilder::GetToolDataClass() {
    return USnapGridFlowToolData::StaticClass();
}

TSubclassOf<UDungeonQuery> USnapGridFlowBuilder::GetQueryClass() {
    return USnapGridFlowQuery::StaticClass();
}

///////////////////////////// USnapGridFlowModel /////////////////////////////
void USnapGridFlowModel::Reset() {
    Connections.Reset();
    ModuleInstances.Reset();
    AbstractGraph = nullptr;
}

///////////////////////////// FSnapGridFlowStreamingChunkHandler /////////////////////////////
FSnapGridFlowStreamingChunkHandler::FSnapGridFlowStreamingChunkHandler(UWorld* InWorld,
        USnapGridFlowModel* InSnapGridModel, UDungeonLevelStreamingModel* InLevelStreamingModel)
    : World(InWorld)
    , SnapGridModel(InSnapGridModel)
    , LevelStreamingModel(InLevelStreamingModel)
{
}

TArray<FSnapConnectionInstance>* FSnapGridFlowStreamingChunkHandler::GetConnections() const {
    return SnapGridModel.IsValid() ? &SnapGridModel->Connections : nullptr;
}

UDungeonLevelStreamingModel* FSnapGridFlowStreamingChunkHandler::GetLevelStreamingModel() const {
    return LevelStreamingModel.Get();
}

UWorld* FSnapGridFlowStreamingChunkHandler::GetWorld() const {
    return World.Get();
}

void FSnapGridFlowStreamingChunkHandler::OnChunkVisible(USnapStreamingChunk* Chunk) {
    FSnapStreamingChunkHandlerBase::OnChunkVisible(Chunk);

    if (Chunk) {
        if (!FullyLoadedChunks.Contains(Chunk->ID)) {
            ChunkFullyLoadedEvent.ExecuteIfBound(Chunk);
        }

        FullyLoadedChunks.Add(Chunk->ID);
    }
}

void FSnapGridFlowStreamingChunkHandler::OnChunkLoaded(USnapStreamingChunk* Chunk) {
    FSnapStreamingChunkHandlerBase::OnChunkLoaded(Chunk);

    ChunkLoadedEvent.ExecuteIfBound(Chunk);
}

void FSnapGridFlowStreamingChunkHandler::OnChunkUnloaded(USnapStreamingChunk* Chunk) {
    FSnapStreamingChunkHandlerBase::OnChunkUnloaded(Chunk);

    if (Chunk) {
        FullyLoadedChunks.Remove(Chunk->ID);
    }
    
    ChunkUnloadedEvent.ExecuteIfBound(Chunk);
}

void FSnapGridFlowStreamingChunkHandler::OnConnectionDoorCreated(FSnapConnectionInstance* ConnectionData) const {
    if (!ConnectionData || !LevelStreamingModel.IsValid() || !SnapGridModel.IsValid() || !SnapGridModel->AbstractGraph) {
        return;
    }
    if (!ConnectionData->bHasSpawnedDoorActor) {
        return;
    }

    UFlowAbstractLink* Link = SnapGridModel->AbstractGraph->GetLink(ConnectionData->ModuleA, ConnectionData->ModuleB, true);
    if (Link && Link->LinkItems.Num() > 0) {
        for (TWeakObjectPtr<AActor> PersistentDoorActor : ConnectionData->SpawnedDoorActors) {
            if (PersistentDoorActor.IsValid()) {
                if (UDungeonFlowItemMetadataComponent* ItemComponent = PersistentDoorActor->FindComponentByClass<UDungeonFlowItemMetadataComponent>()) {
                    ItemComponent->SetFlowItem(Link->LinkItems[0]);
                }
            }
        }
    }
}

void FSnapGridFlowStreamingChunkHandler::UpdateConnectionDoorType(const FSnapConnectionInstance* ConnectionData, USnapConnectionComponent* ConnectionComponent) const {
    if (!ConnectionData) {
        ConnectionComponent->ConnectionState = ESnapConnectionState::Unknown;
        return;
    }

    UFlowAbstractLink* Link = SnapGridModel->AbstractGraph->GetLink(ConnectionData->ModuleA, ConnectionData->ModuleB, true);
    if (!Link) {
        ConnectionComponent->ConnectionState = ESnapConnectionState::Wall;
        return;
    }

    ConnectionComponent->SpawnOffset = FTransform::Identity;
    ConnectionComponent->ConnectionState = ESnapConnectionState::Door;
    ConnectionComponent->DoorType = ESnapConnectionDoorType::NormalDoor;
    
    if (Link->Type == EFlowAbstractLinkType::OneWay) {
        // Check if this is a vertical link
        const FGuid SourceNodeId = Link->SourceSubNode.IsValid() ? Link->SourceSubNode : Link->Source;
        const FGuid DestNodeId = Link->DestinationSubNode.IsValid() ? Link->DestinationSubNode : Link->Destination;

        UGridFlowAbstractGraph3D* Graph = SnapGridModel->AbstractGraph;
        UFlowAbstractNode* SourceNode = Link->SourceSubNode.IsValid() ? Graph->FindSubNode(Link->SourceSubNode) : Graph->GetNode(Link->Source);
        UFlowAbstractNode* DestNode = Link->DestinationSubNode.IsValid() ? Graph->FindSubNode(Link->DestinationSubNode) : Graph->GetNode(Link->Destination);
        check(SourceNode && DestNode);
        
        const float SourceZ = SourceNode->Coord.Z;
        const float DestZ = DestNode->Coord.Z;

        
        if (FMath::IsNearlyEqual(SourceZ, DestZ, 1e-4f)) {
            ConnectionComponent->DoorType = ESnapConnectionDoorType::OneWayDoor;

            // Handle orientation towards the correct one-way direction
            const FVector Forward = ConnectionComponent->GetComponentTransform().GetRotation().GetForwardVector();
            FVector LinkDir = DestNode->Coord - SourceNode->Coord;
            LinkDir.Normalize();

            const float Dot = FVector::DotProduct(Forward, LinkDir);
            // Dot value will be -1 if the vectors point in the opposite direction: https://chortle.ccsu.edu/VectorLessons/vch09/vch09_6.html
            // If this is the case, rotate the spawned one-way door by 180, to fix the direction
            if (Dot < 0) {
                // Points in the opposite direction
                ConnectionComponent->SpawnOffset = FTransform(FQuat(FVector::UpVector, PI));
            }
        }
        else if (SourceZ < DestZ) {
            ConnectionComponent->DoorType = ESnapConnectionDoorType::OneWayDoorUp;
        }
        else if (SourceZ > DestZ) {
            ConnectionComponent->DoorType = ESnapConnectionDoorType::OneWayDoorDown;
        }
    }
    else if (Link->LinkItems.Num() > 0) {
        UFlowGraphItem* Item = Link->LinkItems[0];
        if (Item && Item->ItemType == EFlowGraphItemType::Lock) {
            ConnectionComponent->DoorType = ESnapConnectionDoorType::LockedDoor;
            ConnectionComponent->MarkerName = Item->MarkerName;
        }
    }
}


///////////////////////////// FSnapGridFlowStreamingChunkHandler /////////////////////////////
void FSnapGridFlowProcessDomainExtender::ExtendDomains(FFlowProcessor& InProcessor) {
    // Register the Abstract Layout Graph Domain
    const TSharedPtr<FGridFlowAbstractGraph3DDomain> AbstractGraphDomain = MakeShareable(new FGridFlowAbstractGraph3DDomain);

    if (ModuleDatabase.IsValid()) {
        AbstractGraphDomain->SetGraphConstraints(MakeShareable(new FSnapGridFlowAbstractGraphConstraints(ModuleDatabase.Get())));
        AbstractGraphDomain->SetGroupGenerator(MakeShareable(new FSnapFlowAGNodeGroupGenerator(ModuleDatabase.Get())));
    }
    else {
        AbstractGraphDomain->SetGraphConstraints(MakeShareable(new FNullFlowAbstractGraphConstraints));
        AbstractGraphDomain->SetGroupGenerator(MakeShareable(new FNullFlowAGNodeGroupGenerator));
    }
        
    InProcessor.RegisterDomain(AbstractGraphDomain);

    // Register the snap domain
    InProcessor.RegisterDomain(MakeShareable(new FSnapFlowDomain));    
}

