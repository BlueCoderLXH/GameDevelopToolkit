//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/Lib/FlowAbstractGraphPathUtils.h"

#include "Core/Utils/MathUtils.h"
#include "Core/Utils/StackSystem.h"
#include "Core/Utils/Stats.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphConstraints.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractNode.h"

void FNullFlowAGNodeGroupGenerator::Generate(const FFlowAbstractGraphQuery& InGraphQuery, const UFlowAbstractNode* InCurrentNode,
                const FRandomStream& InRandom, const TSet<FGuid>& InVisited, TArray<FFlowAGPathNodeGroup>& OutGroups) const {
    OutGroups.Reset();
    if (InCurrentNode) {
        FFlowAGPathNodeGroup& Group = OutGroups.AddDefaulted_GetRef();
        Group.bIsGroup = false;
        Group.GroupNodes.Add(InCurrentNode->NodeId);
        Group.GroupEdgeNodes.Add(InCurrentNode->NodeId);
    }
}

void FFlowAbstractGraphPathUtils::FinalizePath(const FFlowAGStaticGrowthState& StaticState, FFlowAGGrowthState& State) {
    TArray<FFlowAGGrowthState_PathItem>& Path = State.Path;

    if (Path.Num() == 0) {
        return;
    }
    
    // Create merged node groups
    for (const FFlowAbstractNodeGroup& GroupInfo : State.NodeGroups) {
        FFlowAbstractGraphPathUtils::CreateMergedCompositeNode(*StaticState.GraphQuery, GroupInfo);
    }
    
    UFlowAbstractGraphBase* Graph = StaticState.GraphQuery->GetGraph<UFlowAbstractGraphBase>();
    TMap<FGuid, FGuid> ChildToParentMap; // [ChildNodeId -> ParentNodeId]
    for (UFlowAbstractNode* ParentNode : Graph->GraphNodes) {
        if (ParentNode->MergedCompositeNodes.Num() > 1) {
            for (UFlowAbstractNode* ChildNode : ParentNode->MergedCompositeNodes) {
                FGuid& ParentId = ChildToParentMap.FindOrAdd(ChildNode->NodeId);
                ParentId = ParentNode->NodeId;
            }
        }
    }

    const int32 PathLength = Path.Num();
    for (int i = 0; i < PathLength; i++) {
        FFlowAGGrowthState_PathItem& PathItem = Path[i];
        const FGuid OrigNodeId = PathItem.NodeId;
        const FGuid OrigPrevNodeId = PathItem.PreviousNodeId;
        if (FGuid* ParentNodeID = ChildToParentMap.Find(PathItem.NodeId)) {
            PathItem.NodeId = *ParentNodeID;
        }
        if (FGuid* ParentNodeID = ChildToParentMap.Find(PathItem.PreviousNodeId)) {
            PathItem.PreviousNodeId = *ParentNodeID;
        }
        
        UFlowAbstractNode* PathNode = StaticState.GraphQuery->GetNode(PathItem.NodeId);
        if (!PathNode) continue;
        PathNode->bActive = true;
        PathNode->Color = StaticState.NodeColor;
        PathNode->PathIndex = i;
        PathNode->PathLength = PathLength;

        FString PathName;
        if (i == 0 && StaticState.StartNodePathNameOverride.Len() > 0) {
            PathName = StaticState.StartNodePathNameOverride;
        }
        else if (i == Path.Num() - 1 && StaticState.EndNodePathNameOverride.Len() > 0) {
            PathName = StaticState.EndNodePathNameOverride;
        }
        else {
            PathName = StaticState.PathName;
        }
        PathNode->PathName = PathName;
        

        // Link the path nodes
        if (i > 0) {
            const FGuid LinkSrc = PathItem.PreviousNodeId;
            const FGuid LinkDst = PathItem.NodeId;
            const FGuid LinkSrcSub = OrigPrevNodeId;
            const FGuid LinkDstSub = OrigNodeId;
            TArray<UFlowAbstractLink*> PossibleLinks = StaticState.Graph->GetLinks(LinkSrc, LinkDst, true);
            UFlowAbstractLink* TargetLink = nullptr;
            for (UFlowAbstractLink* PossibleLink : PossibleLinks) {
                if (!PossibleLink) continue;
                if (PossibleLink->Source == LinkSrc && PossibleLink->Destination == LinkDst) {
                    bool bValid = (!PossibleLink->SourceSubNode.IsValid() || PossibleLink->SourceSubNode == LinkSrcSub);
                    bValid &= (!PossibleLink->DestinationSubNode.IsValid() || PossibleLink->DestinationSubNode == LinkDstSub);

                    // Found the correct link
                    if (bValid) {
                        PossibleLink->Type = EFlowAbstractLinkType::Connected;
                        break;
                    }
                }
                else if (PossibleLink->Source == LinkDst && PossibleLink->Destination == LinkSrc) {
                    bool bValid = (!PossibleLink->SourceSubNode.IsValid() || PossibleLink->SourceSubNode == LinkDstSub);
                    bValid &= (!PossibleLink->DestinationSubNode.IsValid() || PossibleLink->DestinationSubNode == LinkSrcSub);

                    // Found the correct link
                    if (bValid) {
                        PossibleLink->Type = EFlowAbstractLinkType::Connected;
                        PossibleLink->ReverseDirection();
                        break;
                    }
                }
            }
            
            /*
            if (Link) {
                Link->Type = EFlowAbstractLinkType::Connected;
                if (Link->Source == LinkDst && Link->Destination == LinkSrc) {
                    Link->ReverseDirection();
                }
            }
            */
        }
    }

    // Setup the start / end links
    if (StaticState.HeadNode) {
        const FGuid LinkSrc = StaticState.HeadNode->NodeId;
        const FGuid LinkDst = Path[0].NodeId;
        UFlowAbstractLink* HeadLink = StaticState.Graph->GetLink(LinkSrc, LinkDst, true);
        if (HeadLink) {
            HeadLink->Type = EFlowAbstractLinkType::Connected;
            if (HeadLink->Source == LinkDst && HeadLink->Destination == LinkSrc) {
                HeadLink->ReverseDirection();
            }
        }
    }

    // Find the end node, if any so that it can merge back to the specified branch (specified in variable EndOnPath)
    if (State.TailNode.IsValid()) {
        const FGuid LinkSrc = Path[Path.Num() - 1].NodeId;
        const FGuid LinkDst = State.TailNode->NodeId;
        UFlowAbstractLink* TailLink = StaticState.Graph->GetLink(LinkSrc, LinkDst, true);
        if (TailLink) {
            TailLink->Type = EFlowAbstractLinkType::Connected;
            if (TailLink->Source == LinkDst && TailLink->Destination == LinkSrc) {
                TailLink->ReverseDirection();
            }
        }
    }
}

UFlowAbstractNode* FFlowAbstractGraphPathUtils::CreateMergedCompositeNode(FFlowAbstractGraphQuery& GraphQuery, const FFlowAbstractNodeGroup& NodeGroup) {
    if (NodeGroup.GroupNodes.Num() <= 1) {
        return nullptr;
    }
    
    UFlowAbstractGraphBase* Graph = GraphQuery.GetGraph<UFlowAbstractGraphBase>();
    TSet<UFlowAbstractNode*> SubNodes;
    TSet<FGuid> SubNodeIds;
    TSet<UFlowGraphItem*> SubItems;
    
    FVector PreviewLocation = FVector::ZeroVector;
    FVector Coord = FVector::ZeroVector;
    for (const FGuid& SubNodeId : NodeGroup.GroupNodes) {
        UFlowAbstractNode* SubNode = Graph->GetNode(SubNodeId);
        if (!SubNode) continue;
        SubNodes.Add(SubNode);
        SubNodeIds.Add(SubNodeId);
        SubItems.Append(SubNode->NodeItems);
        Coord += SubNode->Coord;
        PreviewLocation += SubNode->PreviewLocation;
    }
    const int32 NumSubNodes = SubNodes.Num();
    if (NumSubNodes > 0) {
        Coord /= NumSubNodes;
        PreviewLocation /= NumSubNodes;

        UFlowAbstractNode* NewNode = Graph->CreateNode();
        NewNode->bActive = true;
        NewNode->NodeItems = SubItems.Array();
        NewNode->Coord = Coord;
        NewNode->PreviewLocation = PreviewLocation;
        NewNode->MergedCompositeNodes = SubNodes.Array();

        // Remove all the sub nodes from the graph 
        for (UFlowAbstractNode* SubNode : SubNodes) {
            Graph->GraphNodes.Remove(SubNode);
        }

        for (UFlowAbstractLink* Link : Graph->GraphLinks) {
            if (SubNodeIds.Contains(Link->Source)) {
                Link->SourceSubNode = Link->Source;
                Link->Source = NewNode->NodeId;
            }
            if (SubNodeIds.Contains(Link->Destination)) {
                Link->DestinationSubNode = Link->Destination;
                Link->Destination = NewNode->NodeId;
            }
        }
        
        Graph->GraphLinks = Graph->GraphLinks.FilterByPredicate([](const UFlowAbstractLink* Link) { return Link->Source != Link->Destination; });
        GraphQuery.Rebuild();
        return NewNode;
    }

    return nullptr;
}

//////////////////////////////////////// FFlowAGPathStackGrowthTask ////////////////////////////////////////

void FFlowAGPathStackGrowthTask::Execute(const FFlowAGPathStackFrame& InFrameState, const FFlowAGStaticGrowthState& StaticState,
                                         FlowPathGrowthSystem& StackSystem) {

    check(StaticState.MinPathSize > 0 && StaticState.MaxPathSize > 0);
    check(StaticState.GraphQuery);

    const FFlowAGGrowthState& State = InFrameState.State;
    const UFlowAbstractNode* CurrentNode = InFrameState.CurrentNode.Get();
    const UFlowAbstractNode* IncomingNode = InFrameState.IncomingNode.Get();

    const int32 PathIndex = State.Path.Num();
    const int32 PathLength = FMath::Clamp(PathIndex + 1, StaticState.MinPathSize, StaticState.MaxPathSize);
    if (PathIndex == 0 && StaticState.HeadNode) {
        // Check if we can connect from the head node to this node
        if (!StaticState.GraphConstraint->IsValid(*StaticState.GraphQuery, StaticState.HeadNode, {CurrentNode}, StaticState.TaskExtenders)) {
            return;
        }
    }

    if (StaticState.NodeCreationConstraint.IsValid()) {
        if (!StaticState.NodeCreationConstraint->CanCreateNodeAt(CurrentNode, PathLength, PathIndex)) {
            return;
        }
    }

    bool bFirstNodeInPath = (PathIndex == 0);

    TArray<FFAGConstraintsLink> BaseIncomingConstraintLinks;
    if (bFirstNodeInPath && StaticState.HeadNode) {
        UFlowAbstractNode* HeadSubNode = StaticState.HeadNode;
        if (StaticState.HeadNode->MergedCompositeNodes.Num() > 1) {
            for (UFlowAbstractLink* GraphLink : StaticState.Graph->GraphLinks) {
                if (GraphLink->Type != EFlowAbstractLinkType::Unconnected) continue;
                if (GraphLink->Source == CurrentNode->NodeId && GraphLink->Destination == StaticState.HeadNode->NodeId) {
                    HeadSubNode = StaticState.GraphQuery->GetSubNode(GraphLink->DestinationSubNode);
                    check(HeadSubNode);
                    break;
                }
                else if (GraphLink->Source == StaticState.HeadNode->NodeId && GraphLink->Destination == CurrentNode->NodeId) {
                    HeadSubNode = StaticState.GraphQuery->GetSubNode(GraphLink->SourceSubNode);
                    check(HeadSubNode);
                    break;
                }
            }
        }
        BaseIncomingConstraintLinks.Add({CurrentNode, HeadSubNode});
    }
    if (IncomingNode) {
        BaseIncomingConstraintLinks.Add({CurrentNode, IncomingNode});
    }

    TArray<FFlowAGPathNodeGroup> PossibleNodeGroups;
    check(StaticState.NodeGroupGenerator.IsValid());
    StaticState.NodeGroupGenerator->Generate(*StaticState.GraphQuery, CurrentNode, *StaticState.Random, State.Visited, PossibleNodeGroups);

    FMathUtils::Shuffle(PossibleNodeGroups, *StaticState.Random);

    {
        float SelectionThreshold = StaticState.Random->FRand();
        TArray<FFlowAGPathNodeGroup> HiPriorityGroups;
        for (int i = 0; i < PossibleNodeGroups.Num(); i++) {
            if (PossibleNodeGroups[i].Weight > SelectionThreshold) {
                HiPriorityGroups.Add(PossibleNodeGroups[i]);
                PossibleNodeGroups.RemoveAt(i);
                i--;
            }
        }
        PossibleNodeGroups.Append(HiPriorityGroups);
    }
    
    for (const FFlowAGPathNodeGroup& GrowthNodeGroup : PossibleNodeGroups) {
        // Check if we can use this newly created group node by connecting in to it
        if (!StaticState.GraphConstraint->IsValid(*StaticState.GraphQuery, GrowthNodeGroup, PathIndex, PathLength, BaseIncomingConstraintLinks,
                                                  StaticState.TaskExtenders)) continue;

        FFlowAGGrowthState NextState = State;

        // Update the frame path and visited state
        NextState.Visited.Append(GrowthNodeGroup.GroupNodes);
        FFlowAGGrowthState_PathItem PathFrame;
        PathFrame.NodeId = CurrentNode->NodeId;
        PathFrame.PreviousNodeId = IncomingNode ? IncomingNode->NodeId : FGuid();
        NextState.Path.Push(PathFrame);

        // Add path node group info
        if (GrowthNodeGroup.bIsGroup) {
            FFlowAbstractNodeGroup NodeGroup;
            NodeGroup.GroupId = FGuid::NewGuid();
            NodeGroup.GroupNodes = GrowthNodeGroup.GroupNodes;
            NextState.NodeGroups.Push(NodeGroup);
        }

        // Check if we reached the desired path size
        if (NextState.Path.Num() >= StaticState.MinPathSize) {
            // Check if we are near the sink node, if any
            if (StaticState.SinkNodes.Num() == 0) {
                // No sink nodes defined.
                StackSystem.FinalizeResult({ NextState, StaticState });
                return;
            }

            {
                TArray<int32> SinkNodeIndices = FMathUtils::GetShuffledIndices(StaticState.SinkNodes.Num(), *StaticState.Random);
                const TArray<int32> GroupEdgeNodeIndices = FMathUtils::GetShuffledIndices(GrowthNodeGroup.GroupEdgeNodes.Num(), *StaticState.Random);
                for (int32 GroupEdgeNodeIndex : GroupEdgeNodeIndices) {
                    const FGuid& GroupEdgeNodeId = GrowthNodeGroup.GroupEdgeNodes[GroupEdgeNodeIndex];
                    TArray<FGuid> ConnectedNodeIds = StaticState.Graph->GetConnectedNodes(GroupEdgeNodeId);
                    const TArray<int32> ConnectedNodeIndices = FMathUtils::GetShuffledIndices(ConnectedNodeIds.Num(), *StaticState.Random);
                    for (int32 ConnectedNodeIndex : ConnectedNodeIndices) {
                        const FGuid& ConnectedNodeId = ConnectedNodeIds[ConnectedNodeIndex];
                        UFlowAbstractNode* ConnectedNode = StaticState.GraphQuery->GetNode(ConnectedNodeId);
                        for (int32 SinkNodeIndex : SinkNodeIndices) {
                            UFlowAbstractNode* SinkNode = StaticState.SinkNodes[SinkNodeIndex].Get();
                            if (!SinkNode) continue;

                            if (NextState.Path.Num() == 1 && SinkNode == StaticState.HeadNode) {
                                // If the path node size is 1, we don't want to connect back to the head node
                                continue;
                            }
                            if (ConnectedNode == SinkNode) {
                                UFlowAbstractNode* GroupEdgeNode = StaticState.GraphQuery->GetNode(GroupEdgeNodeId);
                                // TODO: Iterate through the edge nodes and check if we can connect to the tail node
                                TArray<FFAGConstraintsLink> IncomingConstraintLinks = BaseIncomingConstraintLinks;
                                UFlowAbstractNode* ConnectedSubNode = ConnectedNode;
                                if (ConnectedNode->MergedCompositeNodes.Num() > 1) {
                                    for (UFlowAbstractLink* GraphLink : StaticState.Graph->GraphLinks) {
                                        if (GraphLink->Type != EFlowAbstractLinkType::Unconnected) continue;
                                        if (GraphLink->Source == GroupEdgeNodeId && GraphLink->Destination == ConnectedNodeId) {
                                            ConnectedSubNode = StaticState.GraphQuery->GetSubNode(GraphLink->DestinationSubNode);
                                            check(ConnectedSubNode);
                                            break;
                                        }
                                        else if (GraphLink->Source == ConnectedNodeId && GraphLink->Destination == GroupEdgeNodeId) {
                                            ConnectedSubNode = StaticState.GraphQuery->GetSubNode(GraphLink->SourceSubNode);
                                            check(ConnectedSubNode);
                                            break;
                                        }
                                    }
                                }
                                IncomingConstraintLinks.Add({GroupEdgeNode, ConnectedSubNode});
                                if (!StaticState.GraphConstraint->IsValid(*StaticState.GraphQuery, GrowthNodeGroup, PathIndex, PathLength,
                                                                          IncomingConstraintLinks, StaticState.TaskExtenders)) continue;

                                TArray<const UFlowAbstractNode*> SinkIncomingNodes = {GroupEdgeNode};
                                if (SinkNode == StaticState.HeadNode) {
                                    // The sink and the head are the same. Add the first node to the connected list
                                    UFlowAbstractNode* FirstNodeInPath = StaticState.GraphQuery->GetNode(NextState.Path[0].NodeId);
                                    if (FirstNodeInPath) {
                                        SinkIncomingNodes.Add(FirstNodeInPath);
                                    }
                                }

                                if (!StaticState.GraphConstraint->IsValid(*StaticState.GraphQuery, SinkNode, SinkIncomingNodes, StaticState.TaskExtenders))
                                    continue;

                                NextState.TailNode = SinkNode;
                                StackSystem.FinalizeResult({ NextState, StaticState });
                                return;
                            }
                        }
                    }
                }
            }

            if (NextState.Path.Num() == StaticState.MaxPathSize) {
                // no sink nodes nearby and we've reached the max path size
                return;
            }
        }


        // Try to grow from each outgoing node
        {
            const TArray<int32> GroupEdgeNodeIndices = FMathUtils::GetShuffledIndices(GrowthNodeGroup.GroupEdgeNodes.Num(), *StaticState.Random);
            for (int32 GroupEdgeNodeIndex : GroupEdgeNodeIndices) {
                const FGuid& GroupEdgeNodeId = GrowthNodeGroup.GroupEdgeNodes[GroupEdgeNodeIndex];
                TArray<FGuid> ConnectedNodeIds = StaticState.Graph->GetConnectedNodes(GroupEdgeNodeId);
                const TArray<int32> ConnectedNodeIndices = FMathUtils::GetShuffledIndices(ConnectedNodeIds.Num(), *StaticState.Random);
                for (int32 ConnectedNodeIndex : ConnectedNodeIndices) {
                    const FGuid& ConnectedNodeId = ConnectedNodeIds[ConnectedNodeIndex];
                    if (NextState.Visited.Contains(ConnectedNodeId)) continue;

                    UFlowAbstractNode* ConnectedNode = StaticState.GraphQuery->GetNode(ConnectedNodeId);
                    if (!ConnectedNode) continue;
                    if (ConnectedNode->bActive) continue;
                    UFlowAbstractNode* GroupEdgeNode = StaticState.GraphQuery->GetNode(GroupEdgeNodeId);

                    TArray<FFAGConstraintsLink> IncomingConstraintLinks = BaseIncomingConstraintLinks;
                    IncomingConstraintLinks.Add({GroupEdgeNode, ConnectedNode});
                    if (!StaticState.GraphConstraint->IsValid(*StaticState.GraphQuery, GrowthNodeGroup, PathIndex, PathLength, IncomingConstraintLinks,
                                                              StaticState.TaskExtenders)) {
                        continue;
                    }

                    FFlowAGPathStackFrame NextFrame;
                    NextFrame.State = NextState;
                    NextFrame.CurrentNode = ConnectedNode;
                    NextFrame.IncomingNode = GroupEdgeNode;
                    StackSystem.PushFrame(NextFrame);
                }
            }
        }
    }
}

//////////////////////////////////////////// FFlowAGPathingSystem ////////////////////////////////////////////

void FFlowAGPathingSystem::RegisterGrowthSystem(const UFlowAbstractNode* StartNode, const FFlowAGStaticGrowthState& StaticState, const int32 Count) {
    check(Count > 0);

    for (int i = 0; i < Count; i++) {
        FFlowAGPathStackFrame InitFrame;
        InitFrame.CurrentNode = StartNode;
        InitFrame.IncomingNode = nullptr;
        FlowPathGrowthSystemPtr GrowthSystem = MakeShareable(new FlowPathGrowthSystem(StaticState));
        GrowthSystem->Initialize(InitFrame);
        GrowthSystems.Add(GrowthSystem);
    }
}

void FFlowAGPathingSystem::Execute(int32 StartIdx, int32 EndIdx) {
    bool bRunning = true;
    while (bRunning && !bTimeout && !bFoundResult) {
        bRunning = false;
        for (int i = StartIdx; i <= EndIdx; i++) {
            FlowPathGrowthSystemPtr GrowthSystem = GrowthSystems[i];
            if (GrowthSystem->IsRunning()) {
                GrowthSystem->ExecuteStep();
                bRunning |= GrowthSystem->IsRunning();
                if (GrowthSystem->FoundResult()) {
                    bFoundResult = true;
                    Result = GrowthSystem->GetResult();
                    break;
                }

                FrameCounter++;
                if (FrameCounter >= MaxFramesToProcess) {
                    bTimeout = true;
                    break;
                }
            }
        }
    }
}

void FFlowAGPathingSystem::Execute(int32 InNumParallelSearches) {
    InNumParallelSearches = FMath::Max(InNumParallelSearches, 1);
    
    FrameCounter = 0;
    for (int i = 0; i < GrowthSystems.Num(); i += InNumParallelSearches) {
        const int32 StartIdx = i;
        const int32 EndIdx = FMath::Min(i + InNumParallelSearches - 1, GrowthSystems.Num() - 1);
        Execute(StartIdx, EndIdx);
        if (bFoundResult || bTimeout) {
            break;
        }
    }
}

