//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraph_DebugGrammar.h"

#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraphNode_DebugGrammarNode.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"
#include "Frameworks/Snap/Lib/Utils/SnapDiagnostics.h"

#include "EdGraph/EdGraphPin.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlowEdDebugGrammar, Log, All);

UEdGraph_DebugGrammar::UEdGraph_DebugGrammar() {
    Diagnostics = MakeShareable(new SnapLib::FDiagnostics);
}

void UEdGraph_DebugGrammar::ResetDebugState() {
    Diagnostics->Clear();
    State = FFlowDebugGraphExecState();
    NodeMap.Reset();
}

void UEdGraph_DebugGrammar::OnDebugGraphGenerated() {
    // Create the door nodes
    TArray<class UEdGraphNode*> ExistingNodes = Nodes;
    for (UEdGraphNode* Node : ExistingNodes) {
        if (UEdGraphNode_DebugGrammarNode* DebugNode = Cast<UEdGraphNode_DebugGrammarNode>(Node)) {
            for (UEdGraphPin* OtherNodePin : DebugNode->GetOutputPin()->LinkedTo) {
                UEdGraphNode_DebugGrammarNode* OtherDebugNode = Cast<UEdGraphNode_DebugGrammarNode>(
                    OtherNodePin->GetOwningNode());
                if (OtherDebugNode) {
                    // Create a new door node
                    UEdGraphNode_DebugGrammarDoorNode* DoorNode = NewObject<UEdGraphNode_DebugGrammarDoorNode>(this);
                    Node->NodeGuid = FGuid::NewGuid();
                    DoorNode->Incoming = DebugNode;
                    DoorNode->Outgoing = OtherDebugNode;
                    AddNode(DoorNode);
                }
            }
        }
    }

    // Cache the nodes
    NodeMap.Reset();
    for (UEdGraphNode* Node : Nodes) {
        if (UEdGraphNode_DebugGrammarNode* DebugNode = Cast<UEdGraphNode_DebugGrammarNode>(Node)) {
            NodeMap.Add(DebugNode->NodeId, DebugNode);
        }
    }
}

void UEdGraph_DebugGrammar::ExecuteStep() {
    if (!Diagnostics.IsValid()) {
        return;
    }

    bool bExecuteNextStep = true;
    while (bExecuteNextStep) {
        bExecuteNextStep = false;
        if (State.StepIndex + 1 < Diagnostics->Steps.Num()) {
            State.StepIndex = FMath::Max(0, State.StepIndex + 1);
            const SnapLib::FDiagStep& Step = Diagnostics->Steps[State.StepIndex];
            if (Step.Function == SnapLib::EFunctionType::MoveToNode) {
                TSharedPtr<SnapLib::FPayload_MoveToNode> Payload = StaticCastSharedPtr<SnapLib::FPayload_MoveToNode>(Step.Payload);
                if (Payload.IsValid()) {
                    ProcessMoveToNode(Payload);
                }
                // We don't want to stop here and move to the next one
                bExecuteNextStep = true;
            }
            else if (Step.Function == SnapLib::EFunctionType::BacktrackFromNode) {
                TSharedPtr<SnapLib::FPayload_BacktrackFromNode> Payload = StaticCastSharedPtr<SnapLib::FPayload_BacktrackFromNode>(
                    Step.Payload);
                if (Payload.IsValid()) {
                    ProcessBacktrack(Payload);
                }
            }
            else if (Step.Function == SnapLib::EFunctionType::AssignModule) {
                TSharedPtr<SnapLib::FPayload_AssignModule> Payload = StaticCastSharedPtr<SnapLib::FPayload_AssignModule>(Step.Payload);
                if (Payload.IsValid()) {
                    ProcessAssignModule(Payload);
                }
            }
            else if (Step.Function == SnapLib::EFunctionType::RejectModule) {
                TSharedPtr<SnapLib::FPayload_RejectModule> Payload = StaticCastSharedPtr<SnapLib::FPayload_RejectModule>(Step.Payload);
                if (Payload.IsValid()) {
                    ProcessRejectModule(Payload);
                }
            }
        }
    }
}

void UEdGraph_DebugGrammar::ProcessMoveToNode(TSharedPtr<SnapLib::FPayload_MoveToNode> InPayload) {
    State.NodeStack.Push(InPayload->NodeId);

    ClearCurrentNodeStatusMessage();
    UpdateCurrentNodeFromStack();
    SetCurrentNodeStatusMessage("Move to Node");
    if (State.CurrentNode.IsValid()) {
        State.CurrentNode->bProcessed = true;
    }
}

void UEdGraph_DebugGrammar::ProcessBacktrack(TSharedPtr<SnapLib::FPayload_BacktrackFromNode> InPayload) {
    State.NodeStack.Pop();

    ClearCurrentNodeStatusMessage();
    UpdateCurrentNodeFromStack();
    SetCurrentNodeStatusMessage("Backtrack");

    if (!InPayload->bSuccess) {
        // Invalidate the processed state for the entire subtree (excluding the backtracked node we are on)
        if (State.CurrentNode.IsValid()) {
            UEdGraph_DebugGrammar* DebugGraph = this;
            TraverseSubTree(State.CurrentNode.Get(), false, [DebugGraph](UEdGraphNode_DebugGrammarNode* ChildNode) {
                if (ChildNode->bModuleAssigned) {
                    DebugGraph->GetOnVisualizeUnloadLevel().ExecuteIfBound(ChildNode->NodeId);
                }

                ChildNode->ResetState();
            });
        }
    }

    HilightCurrentNode(FColor::Blue);
}

void UEdGraph_DebugGrammar::ProcessAssignModule(TSharedPtr<SnapLib::FPayload_AssignModule> InPayload) {
    if (State.CurrentNode.IsValid()) {
        SetCurrentNodeStatusMessage("Assign Module");
        State.CurrentNode->bModuleAssigned = true;
        State.CurrentNode->ModuleLevel = InPayload->ModuleLevel;
        State.CurrentNode->ModuleBounds = InPayload->ModuleBounds;
        State.CurrentNode->WorldTransform = InPayload->WorldTransform;
        State.CurrentNode->IncomingDoorIndex = InPayload->IncomingDoorIndex;
        State.CurrentNode->RemoteDoorIndex = InPayload->RemoteDoorIndex;
        State.CurrentNode->IncomingDoorId = InPayload->IncomingDoorId;
        State.CurrentNode->RemoteDoorId = InPayload->RemoteDoorId;
        State.CurrentNode->RemoteNodeId = InPayload->RemoteNodeId;
        State.CurrentNode->IncomingDoorBounds = InPayload->IncomingDoorBounds;

        OnVisualizeLoadLevel.ExecuteIfBound(State.CurrentNode->NodeId, InPayload->ModuleLevel, InPayload->ModuleBounds,
                                            InPayload->WorldTransform);

        // Open the doors
        if (State.CurrentNode->IncomingDoorIndex != -1 && State.CurrentNode->RemoteDoorIndex != -1) {
            OnVisualizeSetConnectionState.ExecuteIfBound(State.CurrentNode->NodeId, State.CurrentNode->IncomingDoorId,
                                                         true);
            OnVisualizeSetConnectionState.ExecuteIfBound(State.CurrentNode->RemoteNodeId,
                                                         State.CurrentNode->RemoteDoorId, true);
        }

        HilightCurrentNode(FColor::Green);
    }
}

void UEdGraph_DebugGrammar::ProcessRejectModule(TSharedPtr<SnapLib::FPayload_RejectModule> InPayload) {
    if (State.CurrentNode.IsValid()) {
        FString Reason;
        switch (InPayload->Reason) {
        case SnapLib::EModuleRejectReason::BoundsCollide:
            Reason = "Bounds Collide";
            break;

        case SnapLib::EModuleRejectReason::NotEnoughDoorsAvailable:
            Reason = "Not enough doors";
            break;

        case SnapLib::EModuleRejectReason::NoModuleAvailable:
            Reason = "Not modules found";
            break;

        case SnapLib::EModuleRejectReason::CannotBuildSubTree:
            Reason = "Subtree build Failed";
            break;
        }
        SetCurrentNodeStatusMessage("Reject Module\n" + Reason);

        if (State.CurrentNode->bModuleAssigned) {
            // Execute level unload event
            OnVisualizeUnloadLevel.ExecuteIfBound(State.CurrentNode->NodeId);

            // Reset the remote connection to a wall
            OnVisualizeSetConnectionState.ExecuteIfBound(State.CurrentNode->RemoteNodeId,
                                                         State.CurrentNode->RemoteDoorId, false);
        }

        HilightCurrentNode(FColor::Red);

        State.CurrentNode->bModuleAssigned = false;
    }
}

void UEdGraph_DebugGrammar::TraverseSubTree(UEdGraphNode_DebugGrammarNode* RootNode, bool bVisitRootNode,
                                            TFunction<void(UEdGraphNode_DebugGrammarNode*)> Visit) {
    if (!RootNode) return;
    TArray<UEdGraphNode_DebugGrammarNode*> Stack;
    Stack.Push(RootNode);
    while (Stack.Num() > 0) {
        UEdGraphNode_DebugGrammarNode* Top = Stack.Pop();
        if (!Top) continue;

        if (bVisitRootNode || Top != RootNode) {
            Visit(Top);
        }

        for (UEdGraphPin* LinkedToPin : Top->GetOutputPin()->LinkedTo) {
            UEdGraphNode_DebugGrammarNode* Child = Cast<UEdGraphNode_DebugGrammarNode>(LinkedToPin->GetOwningNode());
            if (Child) {
                Stack.Push(Child);
            }
        }
    }

}

void UEdGraph_DebugGrammar::Rewind() {
    State = FFlowDebugGraphExecState();
    ClearCurrentNodeStatusMessage();
    UpdateCurrentNodeFromStack();

    for (UEdGraphNode* Node : Nodes) {
        if (UEdGraphNode_DebugGrammarNode* GrammarNode = Cast<UEdGraphNode_DebugGrammarNode>(Node)) {
            if (GrammarNode->bModuleAssigned) {
                OnVisualizeUnloadLevel.ExecuteIfBound(GrammarNode->NodeId);
            }
            GrammarNode->ResetState();
        }
    }
}

UEdGraphNode_DebugGrammarNode* UEdGraph_DebugGrammar::GetCurrentNode() {
    return State.CurrentNode.Get();
}

void UEdGraph_DebugGrammar::LayoutGraph(int32 HorizontalSpacing, int32 VerticalSpacing) {
    UEdGraph_Grammar::LayoutGraph(HorizontalSpacing, VerticalSpacing);

}

void UEdGraph_DebugGrammar::UpdateCurrentNodeFromStack() {
    bool bFoundNode = false;
    if (State.NodeStack.Num() > 0) {
        FGuid NodeId = State.NodeStack.Top();
        UEdGraphNode_DebugGrammarNode* TopNode = GetNode(NodeId);
        if (TopNode) {
            State.CurrentNode = TopNode;
            bFoundNode = true;
        }
    }

    if (!bFoundNode) {
        State.CurrentNode = nullptr;
    }
}

void UEdGraph_DebugGrammar::ClearCurrentNodeStatusMessage() {
    if (State.CurrentNode.IsValid()) {
        State.CurrentNode->StatusMessage = "";

    }
}

void UEdGraph_DebugGrammar::SetCurrentNodeStatusMessage(const FString& InMessage) {
    if (State.CurrentNode.IsValid()) {
        State.CurrentNode->StatusMessage = InMessage;
    }
}

void UEdGraph_DebugGrammar::HilightCurrentNode(const FColor& InColor) {
    FBox Bounds(ForceInit);
    if (State.CurrentNode.IsValid() && State.CurrentNode->bModuleAssigned) {
        Bounds = State.CurrentNode->ModuleBounds.TransformBy(State.CurrentNode->WorldTransform);
    }
    OnVisualizeSetDebugBox.ExecuteIfBound(Bounds, InColor);
}

UEdGraphNode_DebugGrammarNode* UEdGraph_DebugGrammar::GetNode(const FGuid& InNodeId) {
    TWeakObjectPtr<UEdGraphNode_DebugGrammarNode>* SearchResult = NodeMap.Find(InNodeId);
    if (SearchResult) {
        TWeakObjectPtr<UEdGraphNode_DebugGrammarNode> TopNodePtr = *SearchResult;
        return TopNodePtr.Get();
    }
    return nullptr;
}

