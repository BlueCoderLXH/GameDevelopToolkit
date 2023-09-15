//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdAbstractGraph.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractLink.h"
#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractEdGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractGraphHandler.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodes.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"

#include "GraphEditor.h"

#define LOCTEXT_NAMESPACE "FlowDomainEdAbstractGraph"
DEFINE_LOG_CATEGORY_STATIC(LogDomainEdAbstractGraph, Log, All);

void FFlowDomainEdAbstractGraph::InitializeImpl(TSharedPtr<IDetailsView> PropertyEditor) {
    AbstractGraph = NewObject<UGridFlowAbstractEdGraph>();
    AbstractGraph->OnItemWidgetClicked.BindRaw(this, &FFlowDomainEdAbstractGraph::OnItemWidgetClicked);

    // Create the appearance info
    FGraphAppearanceInfo AppearanceInfo;
    AppearanceInfo.CornerText = LOCTEXT("GridFlowAbstractGraphBranding", "Layout Graph");
    AbstractGraphHandler = MakeShareable(new FGridFlowAbstractGraphHandler);
    AbstractGraphHandler->Bind();
    AbstractGraphHandler->SetPropertyEditor(PropertyEditor);

    AbstractGraphEditor = SNew(SGraphEditor)
        .AdditionalCommands(AbstractGraphHandler->GraphEditorCommands)
        .Appearance(AppearanceInfo)
        .GraphToEdit(AbstractGraph)
        .IsEditable(true)
        .ShowGraphStateOverlay(false)
        .GraphEvents(AbstractGraphHandler->GraphEvents);

    AbstractGraphHandler->SetGraphEditor(AbstractGraphEditor);
    AbstractGraphHandler->OnNodeSelectionChanged.BindRaw(this, &FFlowDomainEdAbstractGraph::OnAbstractNodeSelectionChanged);
}

FFlowDomainEditorTabInfo FFlowDomainEdAbstractGraph::GetTabInfo() const {
    static const FFlowDomainEditorTabInfo TabInfo = {
        TEXT("AbstractGraphTab"),
        LOCTEXT("LayoutGraphTabLabel", "Layout Graph"),
        FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details")
    };

    return TabInfo;
}

TSharedRef<SWidget> FFlowDomainEdAbstractGraph::GetContentWidget() {
    return AbstractGraphEditor.ToSharedRef();
}

void FFlowDomainEdAbstractGraph::Tick(float DeltaTime) {
}

void FFlowDomainEdAbstractGraph::Build(FFlowExecNodeStatePtr State) {
    if (!State.IsValid()) return;
    
    // Clear out the existing abstract graph
    {
        TArray<UEdGraphNode*> OldNodes = AbstractGraph->Nodes;
        for (UEdGraphNode* Node : OldNodes) {
            AbstractGraph->RemoveNode(Node);
        }
    }

    UGridFlowAbstractGraph* ScriptGraph = State->GetState<UGridFlowAbstractGraph>(UFlowAbstractGraphBase::StateTypeID);
    if (!ScriptGraph) {
        return;
    }

    AbstractGraph->ScriptGraph = ScriptGraph;

    TMap<FGuid, UGridFlowAbstractEdGraphNode*> EdNodes;
    for (UFlowAbstractNode* ScriptNode : ScriptGraph->GraphNodes) {
        if (!ScriptNode) {
            continue;
        }
        
        FGraphNodeCreator<UGridFlowAbstractEdGraphNode> EdNodeCreator(*AbstractGraph);
        UGridFlowAbstractEdGraphNode* EdNode = EdNodeCreator.CreateNode();
        EdNodeCreator.Finalize();

        EdNode->ScriptNode = ScriptNode;
        EdNode->NodeGuid = ScriptNode->NodeId;
        EdNode->NodePosX = ScriptNode->PreviewLocation.X;
        EdNode->NodePosY = ScriptNode->PreviewLocation.Y;

        EdNodes.Add(EdNode->NodeGuid, EdNode);
    }

    // Create the links
    for (const UFlowAbstractLink* ScriptLink : ScriptGraph->GraphLinks) {
        UGridFlowAbstractEdGraphNode** SourcePtr = EdNodes.Find(ScriptLink->Source);
        UGridFlowAbstractEdGraphNode** DestPtr = EdNodes.Find(ScriptLink->Destination);
        if (!SourcePtr || !DestPtr) {
            UE_LOG(LogDomainEdAbstractGraph, Warning, TEXT("Failed to create link in abstract graph. Invalid state"));
            continue;
        }

        UGridFlowAbstractEdGraphNode* Source = *SourcePtr;
        UGridFlowAbstractEdGraphNode* Dest = *DestPtr;
        if (!Source || !Dest) continue;

        Source->GetOutputPin()->MakeLinkTo(Dest->GetInputPin());
    }

    AbstractGraph->NotifyGraphChanged();
}

void FFlowDomainEdAbstractGraph::SelectItem(const FGuid& InItemId) const {
    AbstractGraph->SelectedItemId = InItemId;
}

void FFlowDomainEdAbstractGraph::GetAllItems(FFlowExecNodeStatePtr State, TArray<UFlowGraphItem*>& OutItems) {
    if (State.IsValid()) {
        UGridFlowAbstractGraph* AbstractScriptGraph = State->GetState<UGridFlowAbstractGraph>(UFlowAbstractGraphBase::StateTypeID);
        AbstractScriptGraph->GetAllItems(OutItems);
    }
}

void FFlowDomainEdAbstractGraph::ClearAllSelections() const {
    AbstractGraphEditor->ClearSelectionSet();
}

void FFlowDomainEdAbstractGraph::SelectNode(const FVector& InNodeCoord, bool bSelected) const {
    UGridFlowAbstractEdGraphNode* TargetNode = nullptr;
    for (UEdGraphNode* EdNode : AbstractGraph->Nodes) {
        if (UGridFlowAbstractEdGraphNode* AbstractNode = Cast<UGridFlowAbstractEdGraphNode>(EdNode)) {
            FVector NodeCoord = AbstractNode->ScriptNode->Coord;
            if (NodeCoord.Equals(InNodeCoord)) {
                TargetNode = AbstractNode;
            }
        }
    }

    if (TargetNode) {
        AbstractGraphEditor->SetNodeSelection(TargetNode, bSelected);
    }
}

void FFlowDomainEdAbstractGraph::OnItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) const {
    SelectItem(InItemId);
    
    TSharedPtr<IMediator> MediatorPtr = Mediator.IsValid() ? Mediator.Pin() : nullptr;
    if (MediatorPtr.IsValid()) {
        MediatorPtr->OnAbstractItemWidgetClicked(InItemId, bDoubleClicked);
    }
}

void FFlowDomainEdAbstractGraph::OnAbstractNodeSelectionChanged(const TSet<UObject*>& InSelectedObjects) const {
    TArray<UGridFlowAbstractEdGraphNode*> EdNodes;
    for (UObject* SelectedObject : InSelectedObjects) {
        if (UGridFlowAbstractEdGraphNode* EdNode = Cast<UGridFlowAbstractEdGraphNode>(SelectedObject)) {
            EdNodes.Add(EdNode);
        }
    }

    TSharedPtr<IMediator> MediatorPtr = Mediator.IsValid() ? Mediator.Pin() : nullptr;
    if (MediatorPtr.IsValid()) {
        MediatorPtr->OnAbstractNodeSelectionChanged(EdNodes);
    }
}

void FFlowDomainEdAbstractGraph::RecenterView(FFlowExecNodeStatePtr State) {
    if (AbstractGraphEditor.IsValid()) {
        AbstractGraphEditor->ZoomToFit(false);
    }
}

IFlowDomainPtr FFlowDomainEdAbstractGraph::CreateDomain() const {
    return MakeShareable(new FGridFlowAbstractGraphDomain);
}

void FFlowDomainEdAbstractGraph::AddReferencedObjects(FReferenceCollector& Collector) {
    if (AbstractGraph) {
        Collector.AddReferencedObject(AbstractGraph);
    }
}


#undef LOCTEXT_NAMESPACE

