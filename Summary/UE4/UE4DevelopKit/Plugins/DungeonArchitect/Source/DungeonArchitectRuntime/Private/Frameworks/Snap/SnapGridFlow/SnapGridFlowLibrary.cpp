//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowLibrary.h"

#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/Lib/FlowAbstractGraphPathUtils.h"
#include "Frameworks/Flow/Domains/Snap/SnapFlowAbstractGraphSupport.h"

//////////////////////////////////////// Graph Node Impl //////////////////////////////////////////////

DEFINE_LOG_CATEGORY_STATIC(LogSnapGridFlowLib, Log, All);

FSnapGridFlowGraphNode::FSnapGridFlowGraphNode(TWeakObjectPtr<UFlowAbstractGraphBase> InGraph, const FGuid& InNodeID)
    : NodeID(InNodeID)
      , Graph(InGraph) {
}

FName FSnapGridFlowGraphNode::GetCategory() const {
    static const FName Category = "Room";
    return Category;
}

TArray<SnapLib::ISnapGraphNodePtr> FSnapGridFlowGraphNode::GetOutgoingNodes(const FGuid& IncomingNodeId) const {
    TArray<SnapLib::ISnapGraphNodePtr> OutgoingNodes;
    for (UFlowAbstractLink* Link : Graph->GraphLinks) {
        if (Link->Type == EFlowAbstractLinkType::Unconnected) {
            continue;
        }
        if (Link->Source == NodeID && Link->Destination != IncomingNodeId) {
            OutgoingNodes.Add(MakeShareable(new FSnapGridFlowGraphNode(Graph, Link->Destination)));
        }
        else if (Link->Destination == GetNodeID() && Link->Source != IncomingNodeId) {
            OutgoingNodes.Add(MakeShareable(new FSnapGridFlowGraphNode(Graph, Link->Source)));
        }
    }
    return OutgoingNodes;
}

FVector FSnapGridFlowGraphNode::GetNodeCoord() const {
    UFlowAbstractNode* Node = Graph->GetNode(NodeID);
    return Node ? Node->Coord : FVector::ZeroVector;
}


//////////////////////////////////////// Graph Generator //////////////////////////////////////////////

class FSnapGridFlowUtils {
public:
    static FBox GetLayoutNodeBounds(const FVector& LayoutCoord, const FVector& BaseOffset, const FVector& ModuleSize) {
        const FVector TargetMin = BaseOffset + LayoutCoord * ModuleSize;
        return FBox(TargetMin, TargetMin + ModuleSize);
    }
};

bool FSnapGridFlowGraphGenerator::ModuleOccludes(const SnapLib::FModuleNodePtr ModuleNode,
                                                 SnapLib::ISnapGraphNodePtr MissionNode,
                                                 const TArray<FBox>& OcclusionList) {
    const TSharedPtr<FSnapGridFlowGraphNode> LayoutMissionNode = StaticCastSharedPtr<FSnapGridFlowGraphNode
    >(MissionNode);
    const FVector LayoutCoord = LayoutMissionNode->GetNodeCoord();
    const FBox TargetBounds = FSnapGridFlowUtils::GetLayoutNodeBounds(LayoutCoord, BaseOffset, ModuleSize);
    FBox ModuleBounds = ModuleNode->GetModuleBounds();
    ModuleBounds = ModuleBounds.ExpandBy(-StaticState.BoundsContraction);
    return !TargetBounds.IsInside(ModuleBounds);
}

TArray<FTransform> FSnapGridFlowGraphGenerator::GetStartingNodeTransforms(SnapLib::FModuleNodePtr ModuleNode, SnapLib::ISnapGraphNodePtr MissionNode) {
    const TSharedPtr<FSnapGridFlowGraphNode> LayoutMissionNode = StaticCastSharedPtr<FSnapGridFlowGraphNode
    >(MissionNode);
    const FVector LayoutCoord = LayoutMissionNode->GetNodeCoord();

    TArray<FTransform> Result;
    TArray<float> Angles = {0, PI * 0.5f, PI, PI * 1.5f};
    TArray<int32> ShuffledAngleIndices = FMathUtils::GetShuffledIndices(4, StaticState.Random);
    FBox TargetBounds = FSnapGridFlowUtils::GetLayoutNodeBounds(LayoutCoord, BaseOffset, ModuleSize);
    for (int i = 0; i < Angles.Num(); i++) {
        float Angle = Angles[ShuffledAngleIndices[i]];
        FQuat Rotation(FVector::UpVector, Angle);
        FTransform RotTransform = FTransform(Rotation, FVector::ZeroVector);

        const FBox BaseModuleBounds = ModuleNode->GetModuleBounds();
        const FBox RotModuleBounds = BaseModuleBounds.TransformBy(RotTransform);

        FVector Offset = TargetBounds.GetCenter() - RotModuleBounds.GetCenter();
        FTransform TargetTransform(Rotation, Offset);
        Result.Add(TargetTransform);
    }
    return Result;
}

//////////////////////////////////////// Lattice Graph Generator //////////////////////////////////////////////

namespace {
    class FModuleItemFitnessCalculator {
    public:
        FModuleItemFitnessCalculator(const TMap<TSoftObjectPtr<UPlaceableMarkerAsset>, int32>& InModuleMarkers) {
            for (auto& Entry : InModuleMarkers) {
                TSoftObjectPtr<UPlaceableMarkerAsset> MarkerAsset = Entry.Key;
                UPlaceableMarkerAsset const* MarkerAssetPtr = MarkerAsset.LoadSynchronous();
                if (!MarkerAssetPtr) continue;
                
                const int32 Count = Entry.Value;
                ModuleMarkers.Add(MarkerAssetPtr, Count);
            }
        }

        int32 Calculate(const TArray<FString>& InMarkerNames) const {
            TMap<UPlaceableMarkerAsset const*, int32> AvailableMarkers = ModuleMarkers;
            return Solve(InMarkerNames, AvailableMarkers);
        }

    private:
        static int32 Solve(const TArray<FString>& InMarkerNames, TMap<UPlaceableMarkerAsset const*, int32>& AvailableMarkers) {
            int32 NumFailed;
            if (AvailableMarkers.Num() > 0) {
                NumFailed = SolveImpl(InMarkerNames, 0, AvailableMarkers);
                check(NumFailed >= 0);
            }
            else {
                NumFailed = InMarkerNames.Num();
            }

            const int32 FAIL_WEIGHT = 1000000;
            return NumFailed * FAIL_WEIGHT;
        }

        // Returns the no. of items failed in the processed sub tree
        static int32 SolveImpl(const TArray<FString>& InMarkerNames, int32 Index, TMap<UPlaceableMarkerAsset const*, int32>& AvailableMarkers) {
            if (Index == InMarkerNames.Num()) {
                return 0;
            }
            
            check (Index >= 0 || Index < InMarkerNames.Num());

            int32 BestFrameFailCount = InMarkerNames.Num();
            const FString& MarkerName = InMarkerNames[Index];
            for (auto& Entry : AvailableMarkers) {
                UPlaceableMarkerAsset const* AvailableMarkerAsset = Entry.Key;
                int32& Count = Entry.Value;
                const bool bCanAttachHere =  (Count > 0 && AvailableMarkerAsset->MarkerNames.Contains(MarkerName));
                int32 FrameFailCount = bCanAttachHere ? 0 : 1;
                if (bCanAttachHere) {
                    Count--;
                }
                FrameFailCount += SolveImpl(InMarkerNames, Index + 1, AvailableMarkers);
                if (bCanAttachHere) {
                    Count++;
                }

                if (FrameFailCount < BestFrameFailCount) {
                    BestFrameFailCount = FrameFailCount;
                }
                
                if (AvailableMarkerAsset->MarkerNames.Num() == 1 && AvailableMarkerAsset->MarkerNames[0] == MarkerName) {
                    // Faster bailout
                    break;
                }
            } 
            
            return BestFrameFailCount;
        }
        
    private:
        TMap<UPlaceableMarkerAsset const*, int32> ModuleMarkers;
    };
}

bool FSnapGridFlowGraphLatticeGenerator::Generate(UFlowAbstractGraphBase* InGraph, TArray<SnapLib::FModuleNodePtr>& OutModuleNodes) const {
    if (!InGraph || !ModuleDatabase.IsValid()) return false;

    TMap<FGuid, SnapLib::FModuleNodePtr> ModuleNodesById;
    TMap<FGuid, FIntVector> ModuleCoordsById;
    TMap<FGuid, TArray<FSGFModuleAssemblySideCell>> ActiveModuleDoorIndices;
    
    const FRandomStream Random(Settings.Seed);
    const FFlowAbstractGraphQuery GraphQuery(InGraph);
    for (UFlowAbstractNode* const Node : InGraph->GraphNodes) {
        if (!Node->bActive) continue;
        
        FIntVector Coord = FMathUtils::ToIntVector(Node->Coord, true);
        for (UFlowAbstractNode* SubNode : Node->MergedCompositeNodes) {
            if (!SubNode) continue;
            const FIntVector SubCoord = FMathUtils::ToIntVector(SubNode->Coord, true);
            Coord.X = FMath::Min(Coord.X, SubCoord.X);
            Coord.Y = FMath::Min(Coord.Y, SubCoord.Y);
            Coord.Z = FMath::Min(Coord.Z, SubCoord.Z);
        }
        
        FFlowAGPathNodeGroup Group;
        TArray<FFAGConstraintsLink> ConstraintLinks;
        FSnapGridFlowAbstractGraphConstraints::BuildNodeGroup(GraphQuery, Node, {}, Group, ConstraintLinks);

        const FGuid& NodeId = Node->NodeId;
        SnapLib::FModuleNodePtr ModuleNode;
        
        // Build the input node assembly
        FSGFModuleAssembly Assembly;
        FSGFModuleAssemblyBuilder::Build(GraphQuery, Group, ConstraintLinks, Assembly);

        struct FModuleFitCandidate {
            TSharedPtr<FSnapGridFlowGraphModDBItemImpl> ModuleItem;
            FQuat ModuleRotation;
            int32 AssemblyIndex;
            TArray<FSGFModuleAssemblySideCell> DoorIndices;
            int32 Priority = 0;
        };

        TArray<FModuleFitCandidate> FitCandidates;
        
        TSet<FName> CategoryNames;
        UFANodeSnapDomainData* NodeSnapData = Node->FindDomainData<UFANodeSnapDomainData>();
        if (NodeSnapData) {
            CategoryNames = TSet<FName>(NodeSnapData->ModuleCategories);
        }
        else {
            UE_LOG(LogSnapGridFlowLib, Error, TEXT("Snap Domain data missing in the abstract graph node"));
        }
        
        TArray<SnapLib::IModuleDatabaseItemPtr> ModuleItems;
        for (const FName& CategoryName : CategoryNames) {
            ModuleItems.Append(ModuleDatabase->GetCategoryModules(CategoryName));
        }

        TArray<FString> DesiredNodeMarkers;
        for (UFlowGraphItem* NodeItem : Node->NodeItems) {
            if (!NodeItem) continue;
            FString MarkerName = NodeItem->MarkerName.TrimStartAndEnd();
            if (MarkerName.Len() > 0) {
                DesiredNodeMarkers.Add(MarkerName);
            }
        }
        
        bool bChooseModulesWithMinDoors = Random.FRand() < Settings.ModulesWithMinimumDoorsProbability;

        TArray<int32> ModuleIndices = FMathUtils::GetShuffledIndices(ModuleItems.Num(), Random);
        for (int32 ModuleIdx : ModuleIndices) {
            TSharedPtr<FSnapGridFlowGraphModDBItemImpl> ModuleItem = StaticCastSharedPtr<FSnapGridFlowGraphModDBItemImpl>(ModuleItems[ModuleIdx]);
            if (!ModuleItem.IsValid()) continue;

            const FSnapGridFlowModuleDatabaseItem ModuleInfo = ModuleItem->GetItem();
            FModuleItemFitnessCalculator ItemFitnessCalculator(ModuleInfo.AvailableMarkers);
            int32 ItemFitness = ItemFitnessCalculator.Calculate(DesiredNodeMarkers);
            const float ModuleEntryWeight = FMath::Clamp(ModuleInfo.SelectionWeight, 0.0f, 1.0f);
            
            const int32 NumAssemblies = ModuleInfo.RotatedAssemblies.Num();
            TArray<int32> ShuffledAsmIndices = FMathUtils::GetShuffledIndices(NumAssemblies, Random);
            for (int32 AsmIdx : ShuffledAsmIndices) {
                const FSGFModuleAssembly& ModuleAssembly = ModuleInfo.RotatedAssemblies[AsmIdx];
                TArray<FSGFModuleAssemblySideCell> DoorIndices;
                if (ModuleAssembly.CanFit(Assembly, DoorIndices)) {
                    FModuleFitCandidate& Candidate = FitCandidates.AddDefaulted_GetRef();
                    Candidate.ModuleItem = ModuleItem;
                    Candidate.ModuleRotation = FQuat(FVector::UpVector, AsmIdx);
                    Candidate.AssemblyIndex = AsmIdx;
                    Candidate.DoorIndices = DoorIndices;
                    const int32 Gap = 1000;

                    // If we choose 
                    float ConnectionWeight = bChooseModulesWithMinDoors ? (ModuleInfo.Connections.Num() * Gap) : 0;
                    const float ModuleWeight = Random.FRandRange(1 - ModuleEntryWeight, 1.0f) * (Gap - 1);
                    
                    Candidate.Priority = ItemFitness + ConnectionWeight + ModuleWeight;
                }
            }
        }

        if (FitCandidates.Num() == 0) {
            return false;
        }
        
        // Find the best fit
        FModuleFitCandidate* BestFit = nullptr;
        {
            TArray<int32> BestFitIndices;
            int32 BestFitPriority = MAX_int32;
            for (int Idx = 0; Idx < FitCandidates.Num(); Idx++) {
                const FModuleFitCandidate& Candidate = FitCandidates[Idx];
                if (BestFitPriority > Candidate.Priority) {
                    BestFitIndices.Reset();
                    BestFitIndices.Add(Idx);
                    BestFitPriority = Candidate.Priority;
                }
                else if (BestFitPriority == Candidate.Priority) {
                    BestFitIndices.Add(Idx);
                }
            }
            if (BestFitIndices.Num() > 0) {
                const int32 BestFitIdx = BestFitIndices[Random.RandRange(0, BestFitIndices.Num() - 1)];
                BestFit = &FitCandidates[BestFitIdx];
            }
        }
        
        check(BestFit);

        // Register using the best fit candidate
        {
            ModuleNode = BestFit->ModuleItem->CreateModuleNode(Node->NodeId);
            float Angle = PI * 0.5f * BestFit->AssemblyIndex;
            FQuat ModuleRotation = FQuat(FVector::UpVector, Angle);
            const FSnapGridFlowModuleDatabaseItem& ModuleDBItem = BestFit->ModuleItem->GetItem();
            
            const FVector HalfChunkSize = Settings.ChunkSize * FMathUtils::ToVector(ModuleDBItem.NumChunks) * 0.5f;
            ModuleNode->WorldTransform =
                    FTransform(-HalfChunkSize)
                    * FTransform(ModuleRotation)
                    * FTransform(HalfChunkSize)
                    * FTransform(FMathUtils::ToVector(Coord) * Settings.ChunkSize)
                    * Settings.BaseTransform;

            // Add the doors
            TArray<FSGFModuleAssemblySideCell>& DoorIndicesRef = ActiveModuleDoorIndices.FindOrAdd(NodeId);
            DoorIndicesRef = BestFit->DoorIndices;
        }
        
        // Register in lookup
        {
            SnapLib::FModuleNodePtr& ModulePtrRef = ModuleNodesById.FindOrAdd(NodeId);
            ModulePtrRef = ModuleNode;
            FIntVector& CoordRef = ModuleCoordsById.FindOrAdd(NodeId);
            CoordRef = Coord;
        }
    }

    for (auto& Entry : ActiveModuleDoorIndices) {
        const FGuid& ModuleId = Entry.Key;
        TArray<FSGFModuleAssemblySideCell>& DoorSideCells = Entry.Value;
        for (FSGFModuleAssemblySideCell& DoorSideCell : DoorSideCells) {
            for (UFlowAbstractLink const* GraphLink : InGraph->GraphLinks) {
                if (GraphLink->Type == EFlowAbstractLinkType::Unconnected) continue;
                if ((GraphLink->Source == DoorSideCell.NodeId || GraphLink->SourceSubNode == DoorSideCell.NodeId)
                        && (GraphLink->Destination == DoorSideCell.LinkedNodeId || GraphLink->DestinationSubNode == DoorSideCell.LinkedNodeId)) {
                    // Outgoing Node
                    DoorSideCell.LinkId = GraphLink->LinkId;
                }
                else if ((GraphLink->Source == DoorSideCell.LinkedNodeId || GraphLink->SourceSubNode == DoorSideCell.LinkedNodeId)
                        && (GraphLink->Destination == DoorSideCell.NodeId || GraphLink->DestinationSubNode == DoorSideCell.NodeId)) {
                    // Incoming Node
                    DoorSideCell.LinkId = GraphLink->LinkId;
                }
            }
        }
    }
                    
    // Link the module nodes together
    for (UFlowAbstractLink const* GraphLink : InGraph->GraphLinks) {
        if (!GraphLink || GraphLink->Type == EFlowAbstractLinkType::Unconnected) continue;
        
        FGuid SourceId = GraphLink->Source;
        FGuid DestId = GraphLink->Destination;

        FSGFModuleAssemblySideCell const* SrcCell = nullptr;
        FSGFModuleAssemblySideCell const* DstCell = nullptr;
        {
            const TArray<FSGFModuleAssemblySideCell>& SourceDoorCells = ActiveModuleDoorIndices.FindOrAdd(SourceId);
            for (const FSGFModuleAssemblySideCell& SourceDoorCell : SourceDoorCells) {
                if (SourceDoorCell.LinkId == GraphLink->LinkId) {
                    SrcCell = &SourceDoorCell;
                    break;
                }
            } 
            const TArray<FSGFModuleAssemblySideCell>& DestDoorCells = ActiveModuleDoorIndices.FindOrAdd(DestId);
            for (const FSGFModuleAssemblySideCell& DestDoorCell : DestDoorCells) {
                if (DestDoorCell.LinkId == GraphLink->LinkId) {
                    DstCell = &DestDoorCell;
                    break;
                }
            } 
        }

        if (!SrcCell || !DstCell) {
            return false;
        }
        
        if (!ModuleNodesById.Contains(SourceId) || !ModuleNodesById.Contains(DestId)) {
            return false;
        }

        SnapLib::FModuleNodePtr SrcModule = ModuleNodesById[SourceId];
        SnapLib::FModuleNodePtr DstModule = ModuleNodesById[DestId];
        SnapLib::FModuleDoorPtr SrcDoor = SrcModule->Doors[SrcCell->ConnectionIdx];
        SnapLib::FModuleDoorPtr DstDoor = DstModule->Doors[DstCell->ConnectionIdx];

        SrcDoor->ConnectedDoor = DstDoor;
        DstDoor->ConnectedDoor = SrcDoor;

        SrcModule->Outgoing.Add(SrcDoor);
        DstModule->Incoming = DstDoor;
    } 
    

    OutModuleNodes.Reset();
    ModuleNodesById.GenerateValueArray(OutModuleNodes);
    return true;
}


//////////////////////////////////////// Module Database Adapter //////////////////////////////////////////////

SnapLib::FModuleNodePtr FSnapGridFlowGraphModDBItemImpl::CreateModuleNode(const FGuid& InNodeId) {
    SnapLib::FModuleNodePtr Node = MakeShareable(new SnapLib::FModuleNode);
    Node->ModuleInstanceId = InNodeId;
    Node->ModuleDBItem = SharedThis(this);

    for (const FSnapGridFlowModuleDatabaseConnectionInfo& DoorInfo : Item.Connections) {
        SnapLib::FModuleDoorPtr Door = MakeShareable(new SnapLib::FModuleDoor);
        Door->ConnectionId = DoorInfo.ConnectionId;
        Door->ConnectionInfo = DoorInfo.ConnectionInfo;
        Door->ConnectionConstraint = DoorInfo.ConnectionConstraint;
        Door->LocalTransform = DoorInfo.Transform;
        Door->Owner = Node;
        Node->Doors.Add(Door);
    }

    return Node;
}

FSnapGridFlowModuleDatabaseImpl::FSnapGridFlowModuleDatabaseImpl(USnapGridFlowModuleDatabase* ModuleDB) {
    if (ModuleDB) {
        for (const FSnapGridFlowModuleDatabaseItem& ModuleInfo : ModuleDB->Modules) {
            TArray<SnapLib::IModuleDatabaseItemPtr>& CategoryModuleNames = ModulesByCategory.FindOrAdd(ModuleInfo.Category);
            CategoryModuleNames.Add(MakeShareable(new FSnapGridFlowGraphModDBItemImpl(ModuleInfo)));
        }
    }
}

