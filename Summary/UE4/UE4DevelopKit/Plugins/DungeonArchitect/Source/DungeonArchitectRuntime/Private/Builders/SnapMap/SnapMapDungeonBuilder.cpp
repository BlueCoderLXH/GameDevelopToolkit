//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SnapMap/SnapMapDungeonBuilder.h"

#include "Builders/SnapMap/SnapMapAsset.h"
#include "Builders/SnapMap/SnapMapDungeonModel.h"
#include "Builders/SnapMap/SnapMapDungeonQuery.h"
#include "Builders/SnapMap/SnapMapDungeonSelectorLogic.h"
#include "Builders/SnapMap/SnapMapDungeonToolData.h"
#include "Builders/SnapMap/SnapMapDungeonTransformLogic.h"
#include "Core/Dungeon.h"
#include "Core/Volumes/DungeonNegationVolume.h"
#include "Core/Volumes/DungeonThemeOverrideVolume.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/GraphGrammarProcessor.h"
#include "Frameworks/GraphGrammar/Script/GrammarScriptGraph.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"
#include "Frameworks/Snap/Lib/Streaming/SnapStreaming.h"
#include "Frameworks/Snap/Lib/Utils/SnapDiagnostics.h"
#include "Frameworks/Snap/SnapMap/SnapMapLibrary.h"
#include "Frameworks/ThemeEngine/SceneProviders/PooledDungeonSceneProvider.h"

#include "DrawDebugHelpers.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(SnapMapDungeonBuilderLog);

USnapMapDungeonBuilder::USnapMapDungeonBuilder(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
}

void USnapMapDungeonBuilder::BuildNonThemedDungeonImpl(UWorld* World, TSharedPtr<FDungeonSceneProvider> SceneProvider) {
    SnapMapModel = Cast<USnapMapDungeonModel>(model);
    SnapMapConfig = Cast<USnapMapDungeonConfig>(config);

    if (!World || !SnapMapConfig.IsValid() || !SnapMapModel.IsValid()) {
        UE_LOG(SnapMapDungeonBuilderLog, Error, TEXT("Invalid reference passed to the snap builder"));
        return;
    }

    if (!SnapMapConfig->DungeonFlowGraph) {
        UE_LOG(SnapMapDungeonBuilderLog, Error, TEXT("Dungeon Flow asset not specified"));
        return;
    }

    if (!SnapMapConfig->ModuleDatabase) {
        UE_LOG(SnapMapDungeonBuilderLog, Error, TEXT("Module Database asset is not specified"));
        return;
    }

    if (LevelStreamHandler.IsValid()) {
        LevelStreamHandler->ClearStreamingLevels();
        LevelStreamHandler.Reset();
    }
    
    SnapMapModel->Reset();
    PropSockets.Reset();
    
    UDungeonLevelStreamingModel* LevelStreamModel = Dungeon ? Dungeon->LevelStreamingModel : nullptr;
    LevelStreamHandler = MakeShareable(new FSnapMapStreamingChunkHandler(GetWorld(), SnapMapModel.Get(), LevelStreamModel));
    LevelStreamHandler->ClearStreamingLevels();

    if (Diagnostics.IsValid()) {
        Diagnostics->Clear();
    }

    const int32 MaxTries = SnapMapConfig->bSupportBuildRetries ? FMath::Max(1, SnapMapConfig->NumBuildRetries) : 1;
    int32 NumTries = 0;

    TSet<int32> ProcessedSeeds;
    int32 Seed = SnapMapConfig->Seed;
    SnapLib::FModuleNodePtr GraphRootNode = nullptr;
    while (NumTries < MaxTries && !GraphRootNode.IsValid()) {
        ProcessedSeeds.Add(Seed);
        GraphRootNode = GenerateModuleNodeGraph(Seed);
        NumTries++;

        if (!GraphRootNode.IsValid()) {
            FRandomStream SeedGenerator(Seed);
            Seed = (int32)SeedGenerator.FRandRange(0, MAX_flt - 1);
            while (ProcessedSeeds.Contains(Seed)) {
                Seed++;
            }
        }
    }

    if (!GraphRootNode) {
        UE_LOG(SnapMapDungeonBuilderLog, Error, TEXT("Cannot find a valid snap solution"));
        return;
    }
    
    FSnapMapGraphSerializer::Serialize({ GraphRootNode }, SnapMapModel->ModuleInstances, SnapMapModel->Connections);
    FGuid SpawnRoomId = GraphRootNode->ModuleInstanceId;
    FSnapStreaming::GenerateLevelStreamingModel(World, { GraphRootNode }, Dungeon->LevelStreamingModel, USnapStreamingChunk::StaticClass(),
        [this, &SpawnRoomId](UDungeonStreamingChunk* InChunk, SnapLib::FModuleNodePtr Node)
        {
            USnapStreamingChunk* Chunk = Cast<USnapStreamingChunk>(InChunk);
            if (Chunk) {
                Chunk->ModuleTransform = Node->WorldTransform;
                if (Node->ModuleInstanceId == SpawnRoomId) {
                    Chunk->bSpawnRoomChunk = true;
                }
                if (LevelStreamHandler.IsValid()) {
                    LevelStreamHandler->RegisterEvents(Chunk);
                }
            }
        },
        SnapMapConfig->OrderCategory);
}

namespace {
    void PopulateNegationVolumeBounds(ADungeon* InDungeon, TArray<SnapLib::FSnapNegationVolumeState>& OutNegationVolumes) {
        UWorld* World = InDungeon ? InDungeon->GetWorld() : nullptr;
        if (!World) return;

        // Grab the bounds of all the negation volumes
        for (TObjectIterator<ADungeonNegationVolume> NegationVolume; NegationVolume; ++NegationVolume) {
            if (!NegationVolume->IsValidLowLevel() || NegationVolume->IsPendingKill()) {
                continue;
            }
            if (NegationVolume->Dungeon != InDungeon) {
                continue;
            }

            FVector Origin, Extent;
            NegationVolume->GetActorBounds(false, Origin, Extent);

            SnapLib::FSnapNegationVolumeState State;
            State.Bounds = FBox(Origin - Extent, Origin + Extent);
            State.bInverse = NegationVolume->Reversed;

            OutNegationVolumes.Add(State);
        }
    }
}

SnapLib::FModuleNodePtr USnapMapDungeonBuilder::GenerateModuleNodeGraph(int32 InSeed) const {
    UGrammarScriptGraph* MissionGraph = NewObject<UGrammarScriptGraph>(SnapMapModel.Get());
    SnapMapModel->MissionGraph = MissionGraph;

    UGraphGrammar* MissionGrammar = SnapMapConfig->DungeonFlowGraph->MissionGrammar;

    // build the mission graph from the mission grammar rules
    {
        FGraphGrammarProcessor GraphGrammarProcessor;
        GraphGrammarProcessor.Initialize(MissionGraph, MissionGrammar, InSeed);
        GraphGrammarProcessor.Execute(MissionGraph, MissionGrammar);
    }

    SnapLib::FGrowthStaticState StaticState;
    StaticState.Random = random;
    StaticState.BoundsContraction = SnapMapConfig->CollisionTestContraction;
    StaticState.DungeonBaseTransform = Dungeon
                                           ? FTransform(FRotator::ZeroRotator, Dungeon->GetActorLocation())
                                           : FTransform::Identity;
    StaticState.StartTimeSecs = FPlatformTime::Seconds();
    StaticState.MaxProcessingTimeSecs = SnapMapConfig->MaxProcessingTimeSecs;
    StaticState.bAllowModuleRotations = SnapMapConfig->bAllowModuleRotations;
    StaticState.Diagnostics = Diagnostics;

    PopulateNegationVolumeBounds(Dungeon, StaticState.NegationVolumes);

    SnapLib::IModuleDatabasePtr ModDB = MakeShareable(new FSnapMapModuleDatabaseImpl(SnapMapConfig->ModuleDatabase));
    SnapLib::FSnapGraphGenerator GraphGenerator(ModDB, StaticState);
    SnapLib::ISnapGraphNodePtr StartNode = MakeShareable(new FSnapGraphGrammarNode(MissionGraph->FindRootNode()));
    return GraphGenerator.Generate(StartNode);
}

void USnapMapDungeonBuilder::BuildPreviewSnapLayout() {
    SnapMapModel = Cast<USnapMapDungeonModel>(model);
    SnapMapConfig = Cast<USnapMapDungeonConfig>(config);

    UWorld* World = Dungeon ? Dungeon->GetWorld() : nullptr;

    if (!World || !SnapMapConfig.IsValid() || !SnapMapModel.IsValid()) {
        UE_LOG(SnapMapDungeonBuilderLog, Error, TEXT("Invalid reference passed to the snap builder"));
        return;
    }

    if (!SnapMapConfig->DungeonFlowGraph) {
        UE_LOG(SnapMapDungeonBuilderLog, Error, TEXT("Dungeon Flow asset not specified"));
        return;
    }

    const int32 Seed = config->Seed;
    random.Initialize(Seed);
    SnapMapModel->Reset();
    if (LevelStreamHandler.IsValid()) {
        LevelStreamHandler->ClearStreamingLevels();
    }
    PropSockets.Reset();

    const SnapLib::FModuleNodePtr GraphRootNode = GenerateModuleNodeGraph(Seed);
    FSnapMapGraphSerializer::Serialize({ GraphRootNode }, SnapMapModel->ModuleInstances, SnapMapModel->Connections);
}


void USnapMapDungeonBuilder::DestroyNonThemedDungeonImpl(UWorld* World) {
    UDungeonBuilder::DestroyNonThemedDungeonImpl(World);

    SnapMapModel = Cast<USnapMapDungeonModel>(model);
    SnapMapConfig = Cast<USnapMapDungeonConfig>(config);

    if (LevelStreamHandler.IsValid()) {
        LevelStreamHandler->ClearStreamingLevels();
        LevelStreamHandler.Reset();
    }

    SnapMapModel->Reset();

    if (Diagnostics.IsValid()) {
        Diagnostics->Clear();
    }
}

void USnapMapDungeonBuilder::GetSnapConnectionActors(ULevel* ModuleLevel,
                                                        TArray<ASnapConnectionActor*>& OutConnectionActors) {
    OutConnectionActors.Reset();
    for (AActor* Actor : ModuleLevel->Actors) {
        if (!Actor) continue;
        if (ASnapConnectionActor* ConnectionActor = Cast<ASnapConnectionActor>(Actor)) {
            OutConnectionActors.Add(ConnectionActor);
        }
    }
}

void USnapMapDungeonBuilder::SetDiagnostics(TSharedPtr<SnapLib::FDiagnostics> InDiagnostics) {
    Diagnostics = InDiagnostics;
}

void USnapMapDungeonBuilder::DrawDebugData(UWorld* InWorld, bool bPersistent /*= false*/, float LifeTime /*= 0*/) {
    TMap<FGuid, TArray<FSnapConnectionInstance>> ConnectionsByModuleId;
    USnapMapDungeonModel* DModel = Cast<USnapMapDungeonModel>(model);
    if (DModel) {
        for (const FSnapConnectionInstance& Connection : DModel->Connections) {
            TArray<FSnapConnectionInstance>& ModuleConnections = ConnectionsByModuleId.FindOrAdd(Connection.ModuleA);
            ModuleConnections.Add(Connection);
        }
    }

    if (Dungeon && Dungeon->LevelStreamingModel) {
        for (UDungeonStreamingChunk* Chunk : Dungeon->LevelStreamingModel->Chunks) {
            if (!Chunk) continue;

            // Draw the bounds
            FVector Center, Extent;
            Chunk->Bounds.GetCenterAndExtents(Center, Extent);
            DrawDebugBox(InWorld, Center, Extent, FQuat::Identity, FColor::Red, false, -1, 0, 10);

            // Draw the connections to the doors
            float AvgZ = 0;
            const FVector CylinderOffset = FVector(0, 0, 10);
            TArray<FSnapConnectionInstance>& ModuleConnections = ConnectionsByModuleId.FindOrAdd(Chunk->ID);
            for (const FSnapConnectionInstance& Connection : ModuleConnections) {
                FVector Location = Connection.WorldTransform.GetLocation();
                DrawDebugCylinder(InWorld, Location, Location + CylinderOffset, 50, 8, FColor::Red, false, -1, 0, 8);
                AvgZ += Location.Z;
            }

            if (ModuleConnections.Num() > 0) {
                AvgZ /= ModuleConnections.Num();
                FVector CenterPoint = Center;
                CenterPoint.Z = AvgZ;

                DrawDebugCylinder(InWorld, CenterPoint, CenterPoint + CylinderOffset, 80, 16, FColor::Green, false, -1,
                                  0, 4);

                // Draw a point in the center of the room
                for (const FSnapConnectionInstance& Connection : ModuleConnections) {
                    FVector Start = CenterPoint + CylinderOffset / 2.0f;
                    FVector End = Connection.WorldTransform.GetLocation() + CylinderOffset / 2.0f;
                    DrawDebugLine(InWorld, Start, End, FColor::Green, false, -1, 0, 20);
                }

            }
        }
    }
}

TSubclassOf<UDungeonModel> USnapMapDungeonBuilder::GetModelClass() {
    return USnapMapDungeonModel::StaticClass();
}

TSubclassOf<UDungeonConfig> USnapMapDungeonBuilder::GetConfigClass() {
    return USnapMapDungeonConfig::StaticClass();
}

TSubclassOf<UDungeonToolData> USnapMapDungeonBuilder::GetToolDataClass() {
    return USnapMapDungeonToolData::StaticClass();
}

TSubclassOf<UDungeonQuery> USnapMapDungeonBuilder::GetQueryClass() {
    return USnapMapDungeonQuery::StaticClass();
}

bool USnapMapDungeonBuilder::ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint,
                                                      const FTransform& Transform, FQuat& OutRotationOffset) {
    return false;
}

bool USnapMapDungeonBuilder::SupportsProperty(const FName& PropertyName) const {
    if (PropertyName == "Themes") return false;
    if (PropertyName == "MarkerEmitters") return false;
    if (PropertyName == "EventListeners") return false;

    return true;
}

TSharedPtr<class FDungeonSceneProvider> USnapMapDungeonBuilder::CreateSceneProvider(
    UDungeonConfig* Config, ADungeon* pDungeon, UWorld* World) {
    return MakeShareable(new FPooledDungeonSceneProvider(pDungeon, World));
}

bool USnapMapDungeonBuilder::CanBuildDungeon(FString& OutMessage) {
    ADungeon* OuterDungeon = Cast<ADungeon>(GetOuter());
    if (OuterDungeon) {
        SnapMapConfig = Cast<USnapMapDungeonConfig>(OuterDungeon->GetConfig());

        if (!SnapMapConfig.IsValid()) {
            OutMessage = "Dungeon not initialized correctly";
            return false;
        }

        if (!SnapMapConfig->DungeonFlowGraph) {
            OutMessage = "Dungeon Flow asset not assigned";
            return false;
        }

        if (!SnapMapConfig->ModuleDatabase) {
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

bool USnapMapDungeonBuilder::PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics,
                                                   const FPropSocket& socket) {
    return false;
}

FTransform USnapMapDungeonBuilder::PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics,
                                                         const FPropSocket& socket) {
    return FTransform::Identity;
}

///////////////////////////////////// FSnapMapStreamingChunkHandler /////////////////////////////////////
FSnapMapStreamingChunkHandler::FSnapMapStreamingChunkHandler(UWorld* InWorld, USnapMapDungeonModel* InSnapMapModel,
        UDungeonLevelStreamingModel* InLevelStreamingModel)
            : World(InWorld)
            , SnapMapModel(InSnapMapModel)
            , LevelStreamingModel(InLevelStreamingModel)
{
}

TArray<FSnapConnectionInstance>* FSnapMapStreamingChunkHandler::GetConnections() const {
    return SnapMapModel.IsValid() ? &SnapMapModel->Connections : nullptr;
}

UDungeonLevelStreamingModel* FSnapMapStreamingChunkHandler::GetLevelStreamingModel() const {
    return LevelStreamingModel.Get();
}

UWorld* FSnapMapStreamingChunkHandler::GetWorld() const {
    return World.Get();
}

