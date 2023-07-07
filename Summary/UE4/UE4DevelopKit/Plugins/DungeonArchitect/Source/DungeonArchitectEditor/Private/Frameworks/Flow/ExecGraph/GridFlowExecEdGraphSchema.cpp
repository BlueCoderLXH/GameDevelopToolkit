//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/ExecGraph/GridFlowExecEdGraphSchema.h"

#include "Core/Utils/DungeonGraphUtils.h"
#include "Frameworks/Flow/Domains/FlowDomain.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_Initialize.h"
#include "Frameworks/Flow/ExecGraph/GridFlowExecConnectionDrawingPolicy.h"
#include "Frameworks/Flow/ExecGraph/GridFlowExecEdGraph.h"
#include "Frameworks/Flow/ExecGraph/Nodes/GridFlowExecEdGraphNodes.h"
#include "Frameworks/Flow/ExecGraph/Utils/ExecGraphEditorUtils.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "UObject/UObjectIterator.h"

#define LOCTEXT_NAMESPACE "GridFlowExecEdGraphSchema"

UGridFlowExecEdGraphSchema::UGridFlowExecEdGraphSchema(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    DomainFilter = MakeShareable(new FGridFlowExecSchemaDomainFilter);
}

void UGridFlowExecEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const {
    const UGridFlowExecEdGraph* Graph = Cast<UGridFlowExecEdGraph>(ContextMenuBuilder.CurrentGraph);

    TArray<TSharedPtr<FEdGraphSchemaAction>> Actions;
    GetActionList(Actions, Graph, ContextMenuBuilder.OwnerOfTemporaries);

    for (TSharedPtr<FEdGraphSchemaAction> Action : Actions) {
        ContextMenuBuilder.AddAction(Action);
    }
}

namespace {
    template <typename TTask>
    void AddTaskContextAction(const FText& Category, const FText& Title, const FText& Tooltip, int32 Priority,
                              const TArray<IFlowDomainWeakPtr>& Domains, TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, UEdGraph* OwnerOfTemporaries) {
        AddTaskContextAction(TTask::StaticClass(), Category, Title, Tooltip, Priority, Domains, OutActions, OwnerOfTemporaries);
    }

    void AddTaskContextAction(UClass* Class, const FText& Category, const FText& Title, const FText& Tooltip, int32 Priority,
                              const TArray<IFlowDomainWeakPtr>& Domains, TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, UEdGraph* OwnerOfTemporaries) {
        const TSharedPtr<FGridFlowExecSchemaAction_NewNode> NewActorNodeAction = MakeShared<FGridFlowExecSchemaAction_NewNode>(Category, Title, Tooltip, 0, Domains);
        OutActions.Add(NewActorNodeAction);
        
        UGridFlowExecEdGraphNode_Task* TaskNodeTemplate = NewObject<UGridFlowExecEdGraphNode_Task>(OwnerOfTemporaries);
        TaskNodeTemplate->TaskTemplate = NewObject<UFlowExecTask>(TaskNodeTemplate, Class);
        NewActorNodeAction->NodeTemplate = TaskNodeTemplate;
    }
}

void UGridFlowExecEdGraphSchema::GetActionList(TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions,
                                               const UEdGraph* Graph, UEdGraph* OwnerOfTemporaries) const {
    struct FTaskClassInfo {
        UClass* Class;
        FText Title;
        FText Tooltip;
        FText Category;
        int32 Priority;
    };
    TArray<FTaskClassInfo> TaskClasses;
    TArray<IFlowDomainWeakPtr> DomainList;
    if (DomainFilter.IsValid()) {
        for (IFlowDomainWeakPtr DomainPtr : DomainFilter->GetAllowedDomains()) {
            IFlowDomainPtr Domain = DomainPtr.Pin();
            if (Domain.IsValid()) {
                DomainList.Add(Domain);
                
                int32 TaskCounter = 1;
                TArray<UClass*> DomainClasses;
                Domain->GetDomainTasks(DomainClasses);
                const FText DomainCategory = Domain->GetDomainDisplayName();
                for (UClass* Class : DomainClasses) {
                    FTaskClassInfo ClassInfo;
                    ClassInfo.Class = Class;
                    ClassInfo.Category = DomainCategory;
                    FString TitleText = FString::Printf(TEXT("%d.  %s"), TaskCounter++, *Class->GetMetaData("Title"));
                    ClassInfo.Title = FText::FromString(TitleText);
                    ClassInfo.Tooltip = FText::FromString(Class->GetMetaData("Tooltip"));
                    ClassInfo.Priority = Class->GetIntMetaData("MenuPriority");
                    TaskClasses.Add(ClassInfo);
                }
            }
        }
    }
    
    for (const FTaskClassInfo& ClassInfo : TaskClasses) {
        AddTaskContextAction(ClassInfo.Class, ClassInfo.Category, ClassInfo.Title, ClassInfo.Tooltip, ClassInfo.Priority, DomainList, OutActions, OwnerOfTemporaries);
    }
}

void UGridFlowExecEdGraphSchema::SetAllowedDomains(const TArray<IFlowDomainWeakPtr>& InAllowedDomains) const {
    if (DomainFilter.IsValid()) {
        DomainFilter->SetAllowedDomains(InAllowedDomains);
    }
}

TArray<IFlowDomainWeakPtr> UGridFlowExecEdGraphSchema::GetAllowedDomains() const { return DomainFilter->GetAllowedDomains(); }

const FPinConnectionResponse UGridFlowExecEdGraphSchema::CanCreateConnection(
    const UEdGraphPin* A, const UEdGraphPin* B) const {
    // Make sure the data types match
    if (A->PinType.PinCategory != B->PinType.PinCategory) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }
    // Make sure they are not the same pins
    if (A->GetOwningNode() == B->GetOwningNode()) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }

    // Constrain result node direction
    if (A->GetOwningNode()->IsA<UGridFlowExecEdGraphNode_Result>()) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }

    return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

FLinearColor UGridFlowExecEdGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const {
    return FColor::Yellow;
}

bool UGridFlowExecEdGraphSchema::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const {
    return false;
}

UEdGraphNode* UGridFlowExecEdGraphSchema::CreateSubstituteNode(UEdGraphNode* Node, const UEdGraph* Graph,
            FObjectInstancingGraph* InstanceGraph, TSet<FName>& InOutExtraNames) const {
    if (UGridFlowExecEdGraphNode_Task* TaskNode = Cast<UGridFlowExecEdGraphNode_Task>(Node)) {
        if (DomainFilter.IsValid()) {
            for (const IFlowDomainWeakPtr DomainPtr : DomainFilter->GetAllowedDomains()) {
                IFlowDomainPtr Domain = DomainPtr.Pin();
                if (Domain.IsValid()) {
                    UFlowExecTask* CompatibleTemplate =  Domain->TryCreateCompatibleTask(TaskNode->TaskTemplate);
                    if (CompatibleTemplate) {
                        TaskNode->TaskTemplate = CompatibleTemplate;
                        return TaskNode;
                    }
                }
            } 
        }
    }
    
    return nullptr;
}

FConnectionDrawingPolicy* UGridFlowExecEdGraphSchema::CreateConnectionDrawingPolicy(
    int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect,
    class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const {
    return new FGridFlowExecConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect,
                                                    InDrawElements, InGraphObj);
}

#if WITH_EDITOR
bool UGridFlowExecEdGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const {
    UGridFlowExecEdGraphNodeBase* NodeA = Cast<UGridFlowExecEdGraphNodeBase>(A->GetOwningNode());
    UGridFlowExecEdGraphNodeBase* NodeB = Cast<UGridFlowExecEdGraphNodeBase>(B->GetOwningNode());
    UEdGraphPin* OutputA = NodeA->GetOutputPin();
    UEdGraphPin* InputB = NodeB->GetInputPin();
    if (!OutputA || !InputB) {
        return false;
    }

    bool bSourceIsTilemapCreateNode = false;
    {
        if (UGridFlowExecEdGraphNode_Task* TaskNodeA = Cast<UGridFlowExecEdGraphNode_Task>(NodeA)) {
            bSourceIsTilemapCreateNode = TaskNodeA-> TaskTemplate && TaskNodeA->TaskTemplate->IsA<UGridFlowTaskTilemap_Initialize>();
        }
    }
    bool bConnectionMade = UEdGraphSchema::TryCreateConnection(OutputA, InputB);
    if (bConnectionMade && OutputA && InputB) {
        if (!bSourceIsTilemapCreateNode) {
            // Allow only one outgoing link
            TArray<UEdGraphPin*> LinkedPins = A->LinkedTo;
            for (UEdGraphPin* LinkedPin : LinkedPins) {
                if (LinkedPin != InputB) {
                    // Break this pin
                    OutputA->BreakLinkTo(LinkedPin);
                }
            }
            // Break a reverse link, if it exists
            {
                UEdGraphPin* InputA = NodeA->GetInputPin();
                UEdGraphPin* OutputB = NodeB->GetOutputPin();
                if (InputA && OutputB) {
                    OutputB->BreakLinkTo(InputA);
                }
            }

            UEdGraph* Graph = A->GetOwningNode()->GetGraph();
            Graph->NotifyGraphChanged();
        }
    }
    return bConnectionMade;
}

void UGridFlowExecEdGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const {
    UEdGraphSchema::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
    TargetPin.GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UGridFlowExecEdGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const {
    UEdGraphSchema::BreakSinglePinLink(SourcePin, TargetPin);
    SourcePin->GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UGridFlowExecEdGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const {
    UEdGraphSchema::BreakNodeLinks(TargetNode);
    TargetNode.GetGraph()->NotifyGraphChanged();
}
#endif // WITH_EDITOR

//////////////////////////////////////// FFlowExecSchemaAction_NewNode ////////////////////////////////////////
UEdGraphNode* FGridFlowExecSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin,
                                                               const FVector2D Location,
                                                               bool bSelectNewNode /*= true*/) {
    UEdGraphNode* NewNode = FDungeonSchemaAction_NewNode::PerformAction(ParentGraph, FromPin, Location, bSelectNewNode);

    TArray<IFlowDomainPtr> DomainList;
    for (IFlowDomainWeakPtr DomainPtr : Domains) {
        IFlowDomainPtr Domain = DomainPtr.Pin();
        if (Domain.IsValid()) {
            DomainList.Add(Domain);
        }
    }
    
    if (UGridFlowExecEdGraphNode_Task* TaskNode = Cast<UGridFlowExecEdGraphNode_Task>(NewNode)) {
        if (UFlowExecTask* Task = TaskNode->TaskTemplate) {
            Task->Extenders.Reset();
            FExecGraphEditorUtils::AddDomainTaskExtensions(Task, DomainList);
        }
    }

    return NewNode;
}

#undef LOCTEXT_NAMESPACE

