//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_CreatePath.h"

#include "Core/Utils/Attributes.h"
#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractNode.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/Lib/FlowAbstractGraphPathUtils.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

namespace {
	TArray<TWeakObjectPtr<UFlowAbstractNode>> GetNodesOnPath(const FString& PathName, UFlowAbstractGraphBase* Graph) {
		TArray<TWeakObjectPtr<UFlowAbstractNode>> Nodes;
		if (PathName.Len() > 0) {
			for (UFlowAbstractNode* Node : Graph->GraphNodes) {
				if (Node->PathName == PathName) {
					Nodes.Add(Node);
				}
			}
		}
		return Nodes;
	}
}

void UFlowTaskAbstract_CreatePath::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
	if (Input.IncomingNodeOutputs.Num() == 0) {
		Output.ErrorMessage = "Missing Input";
		Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
		return;
	}
	if (Input.IncomingNodeOutputs.Num() > 1) {
		Output.ErrorMessage = "Only one input allowed";
		Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
		return;
	}
	if (MinPathSize <= 0) {
		Output.ErrorMessage = "Invalid Path Size";
		Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
		return;
	}
	if (StartFromPath.Len() == 0) {
		Output.ErrorMessage = "StartFromPath not specified";
		Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
		return;
	}
	if (bEnterThroughTeleporter && TeleporterMarkerName.Len() == 0) {
		Output.ErrorMessage = "Teleporter Marker not specified";
		Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
		return;
	}

	Output.State = Input.IncomingNodeOutputs[0].State->Clone();
	UFlowAbstractGraphBase* Graph = Output.State->GetState<UFlowAbstractGraphBase>(UFlowAbstractGraphBase::StateTypeID);
	FFlowAbstractGraphQuery GraphQuery(Graph);

	const FRandomStream& Random = *Input.Random;

	struct FStartNodeCandidate {
		FGuid StartNodeId;
		TWeakObjectPtr<UFlowAbstractNode> OriginatingHeadNode;
	};

	// Find the Start Node candidates
	TArray<TWeakObjectPtr<UFlowAbstractNode>> SourceNodes = GetNodesOnPath(StartFromPath, Graph);
	TArray<FStartNodeCandidate> PossibleStartNodes;
	if (bEnterThroughTeleporter) {
		// Grab all the free abstract nodes and register them
		for (UFlowAbstractNode* GraphNode : Graph->GraphNodes) {
			if (GraphNode && !GraphNode->bActive) {
				FStartNodeCandidate& StartNodeInfo = PossibleStartNodes.AddDefaulted_GetRef();
				StartNodeInfo.StartNodeId = GraphNode->NodeId;
				StartNodeInfo.OriginatingHeadNode = nullptr;
			}
		}
	}
	else {
		for (TWeakObjectPtr<UFlowAbstractNode> HeadNodePtr : SourceNodes) {
			UFlowAbstractNode* HeadNode = HeadNodePtr.Get();
			if (!HeadNode) continue;

			TArray<FGuid> StartNodeIds = Graph->GetConnectedNodes(HeadNode->NodeId);
			for (const FGuid& StartNodeId : StartNodeIds) {
				FStartNodeCandidate& StartNodeInfo = PossibleStartNodes.AddDefaulted_GetRef();
				StartNodeInfo.StartNodeId = StartNodeId;
				StartNodeInfo.OriginatingHeadNode = HeadNode;
			}
		}
	}

	// Find the sink node candidates
	TArray<TWeakObjectPtr<UFlowAbstractNode>> SinkNodes = GetNodesOnPath(EndOnPath, Graph);

	FFlowAGPathingSystem PathingSystem(MaxFramesToProcess);
	{
		TSet<FGuid> VisitedStartNodes;
		TArray<int32> StartNodeIndices = FMathUtils::GetShuffledIndices(PossibleStartNodes.Num(), Random);
		for (int32 StartNodeIdx : StartNodeIndices) {
			FStartNodeCandidate& StartNodeInfo = PossibleStartNodes[StartNodeIdx];

			const FGuid& StartNodeId = StartNodeInfo.StartNodeId;
			if (VisitedStartNodes.Contains(StartNodeId)) continue;
			VisitedStartNodes.Add(StartNodeId);

			UFlowAbstractNode* StartNode = GraphQuery.GetNode(StartNodeId);
			if (!StartNode) continue;
			if (StartNode->bActive) {
				// Node is already in use
				continue;
			}

			FFlowAGStaticGrowthState StaticState;
			StaticState.Graph = Graph;
			StaticState.GraphQuery = &GraphQuery;
			StaticState.HeadNode = StartNodeInfo.OriginatingHeadNode.Get();
			StaticState.SinkNodes = SinkNodes;
			StaticState.Random = Input.Random;
			StaticState.MinPathSize = MinPathSize;
			StaticState.MaxPathSize = MaxPathSize;
			StaticState.NodeColor = NodeColor;
			StaticState.PathName = PathName;
			StaticState.StartNodePathNameOverride = StartNodePathNameOverride;
			StaticState.EndNodePathNameOverride = EndNodePathNameOverride;
			StaticState.NodeGroupGenerator = CreateNodeGroupGenerator(Input.Domain);
			StaticState.GraphConstraint = CreateGraphConstraints(Input.Domain);

			for (UFlowExecTaskExtender* Extender : Extenders) {
				StaticState.TaskExtenders.Add(Extender);
			}

			check(StaticState.GraphConstraint.IsValid());

			PathingSystem.RegisterGrowthSystem(StartNode, StaticState);
		}
	}
	
	PathingSystem.Execute(NumParallelSearches);
	if (PathingSystem.FoundResult()) {
		FFlowAGPathingSystemResult PathResult = PathingSystem.GetResult();
		FinalizePath(PathResult.StaticState, PathResult.State);

		const FFlowAGGrowthState& State = PathResult.State;
		
		// Create a teleporter, if specified
		if (bEnterThroughTeleporter && State.Path.Num() > 0 && SourceNodes.Num() > 0) {
			TWeakObjectPtr<UFlowAbstractNode> SourceNode = SourceNodes[Random.RandRange(0, SourceNodes.Num() - 1)];
			if (SourceNode.IsValid()) {
				FGuid NodeId1 = SourceNode->NodeId;
				FGuid NodeId2 = State.Path[0].NodeId;

				UFlowAbstractNode* Node1 = GraphQuery.GetNode(NodeId1);
				UFlowAbstractNode* Node2 = GraphQuery.GetNode(NodeId2);
				if (Node1 && Node2) {
					UFlowGraphItem* FirstItem = Node1->CreateNewItem<UFlowGraphItem>();
					FirstItem->ItemType = EFlowGraphItemType::Teleporter;
					FirstItem->MarkerName = TeleporterMarkerName;

					// Create the second item
					UFlowGraphItem* SecondItem = Node2->CreateNewItem<UFlowGraphItem>();
					SecondItem->ItemType = EFlowGraphItemType::Teleporter;
					SecondItem->MarkerName = TeleporterMarkerName;

					// Link the items together
					FirstItem->ReferencedItemIds.Add(SecondItem->ItemId);
					SecondItem->ReferencedItemIds.Add(FirstItem->ItemId);
				}
			}
		}

		Output.ExecutionResult = EFlowTaskExecutionResult::Success;
		return;
	}

	Output.ErrorMessage = "Cannot find path";
	Output.ExecutionResult = EFlowTaskExecutionResult::FailRetry;
}

bool UFlowTaskAbstract_CreatePath::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
	FLOWTASKATTR_GET_INT(MinPathSize);
	FLOWTASKATTR_GET_INT(MaxPathSize);
	FLOWTASKATTR_GET_STRING(PathName);
	FLOWTASKATTR_GET_STRING(StartFromPath);
	FLOWTASKATTR_GET_STRING(EndOnPath);
	FLOWTASKATTR_GET_STRING(StartNodePathNameOverride);
	FLOWTASKATTR_GET_STRING(EndNodePathNameOverride);

	return false;
}

bool UFlowTaskAbstract_CreatePath::SetParameter(const FString& InParameterName, const FDAAttribute& InValue) {
	FLOWTASKATTR_SET_INT(MinPathSize);
	FLOWTASKATTR_SET_INT(MaxPathSize);
	FLOWTASKATTR_SET_STRING(PathName);
	FLOWTASKATTR_SET_STRING(StartFromPath);
	FLOWTASKATTR_SET_STRING(EndOnPath);
	FLOWTASKATTR_SET_STRING(StartNodePathNameOverride);
	FLOWTASKATTR_SET_STRING(EndNodePathNameOverride);

	return false;
}

bool UFlowTaskAbstract_CreatePath::SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) {
	FLOWTASKATTR_SET_PARSE_INT(MinPathSize);
	FLOWTASKATTR_SET_PARSE_INT(MaxPathSize);
	FLOWTASKATTR_SET_PARSE_STRING(PathName);
	FLOWTASKATTR_SET_PARSE_STRING(StartFromPath);
	FLOWTASKATTR_SET_PARSE_STRING(EndOnPath);
	FLOWTASKATTR_SET_PARSE_STRING(StartNodePathNameOverride);
	FLOWTASKATTR_SET_PARSE_STRING(EndNodePathNameOverride);

	return false;
}
