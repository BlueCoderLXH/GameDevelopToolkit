//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Editor/SGrammarRuleGraph.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Frameworks/GraphGrammar/Editor/SEditableListView.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/EdGraph_FlowExec.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecEntryNode.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecRuleNode.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"
#include "Frameworks/GraphGrammar/Script/GrammarRuleScript.h"

#include "EdGraphUtilities.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/Commands/UICommandList.h"
#include "GraphEditor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDetailsView.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/MessageDialog.h"
#include "SDropTarget.h"
#include "SNodePanel.h"
#include "ScopedTransaction.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "GrammarRuleEditor"


/** The list view mode of the asset view */
class DUNGEONARCHITECTEDITOR_API SGrammarRuleGraphDropTarget : public SDropTarget {
public:
    virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override {
        PanelCoordDropPosition = MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
        return SDropTarget::OnDrop(MyGeometry, DragDropEvent);
    }

    FVector2D GetPanelCoordDropPosition() const { return PanelCoordDropPosition; }

private:
    FVector2D PanelCoordDropPosition = FVector2D(0, 0);
};


SGrammarRuleGraph::~SGrammarRuleGraph() {
    // Remove the old graph change handler
    if (Graph.IsValid()) {
        Graph->RemoveOnGraphChangedHandler(GraphChangedHandler);
    }
}

void SGrammarRuleGraph::Construct(const FArguments& InArgs, UEdGraph_Grammar* InGraph) {
    OnDelete = InArgs._OnDelete;
    OnGraphChanged = InArgs._OnGraphChanged;
    Title = InArgs._Title;
    PropertyEditor = InArgs._PropertyEditor;
    DropHandler = InArgs._DropHandler;
    bIsEditable = InArgs._IsEditable;


    Flasher.FlashDuration = 1.5f;
    Flasher.Widget = this->AsShared();
    Flasher.FlashColor = InArgs._BorderFlashColor;

    if (!InArgs._bFullScreen) {
        GraphEditorHost = SNew(SBox)
            .HeightOverride(300);

        TSharedPtr<SWidget> CloseButtonWidget;
        if (InArgs._bShowCloseButton) {
            CloseButtonWidget =
                SNew(SBox)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				.MinDesiredWidth(20)
				.MinDesiredHeight(20)
				.Padding(4.0f)
                [
                    SNew(SButton)
					.ButtonStyle(
                                     &FDungeonArchitectStyle::Get().GetWidgetStyle<FButtonStyle>(
                                         "DungeonArchitect.Button.Close.White"))
					.OnClicked(this, &SGrammarRuleGraph::OnDeleteClicked)
                ];
        }
        else {
            CloseButtonWidget = SNullWidget::NullWidget;
        }
        TSharedPtr<SWidget> WeightWidget;
        if (InArgs._bShowWeight) {
            WeightWidget =
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("WeightLabel", "Weight: "))
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SBox)
                    .MinDesiredWidth(20)
                    [
                        SNew(SEditableTextBox)
						.Text(this, &SGrammarRuleGraph::GetGraphWeightText)
						.OnTextCommitted(this, &SGrammarRuleGraph::OnGraphWeightTextCommit)
						.Padding(4)
                    ]
                ];
        }
        else {
            WeightWidget = SNullWidget::NullWidget;
        }

        TSharedPtr<SWidget> GraphToolbar = SNew(SBorder)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(1, 1, 1, 0.5f))
			.Padding(0.f)
			.Padding(FMargin(4, 4, 4, 4))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.ButtonStyle(
                                 &FDungeonArchitectStyle::Get().GetWidgetStyle<FButtonStyle>(
                                     "DA.SnapEd.Buttons.LinkNonDependent"))
					.OnClicked_Raw(this, &SGrammarRuleGraph::ConvertLinkToNonDependent)
					.ToolTipText(LOCTEXT("GraphToolButton_ConvertNonDependent", "Convert to Non-dependent link"))
					.ContentPadding(2.0f)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.ButtonStyle(
                                 &FDungeonArchitectStyle::Get().GetWidgetStyle<FButtonStyle>(
                                     "DA.SnapEd.Buttons.LinkDependent"))
					.OnClicked_Raw(this, &SGrammarRuleGraph::ConvertLinkToDependent)
					.ToolTipText(LOCTEXT("GraphToolButton_ConvertDependent", "Convert to Dependent link"))
					.ContentPadding(2.0f)
            ]
        ];

        TSharedPtr<SWidget> GraphEditorOverlay = SNew(SOverlay)
            + SOverlay::Slot()
            [
                GraphEditorHost.ToSharedRef()
            ]
            + SOverlay::Slot()
              .HAlign(HAlign_Left)
              .VAlign(VAlign_Top)
            [
                GraphToolbar.ToSharedRef()
            ];

        if (DropHandler.IsValid()) {
            GraphEditorOverlay = WrapDropTarget(GraphEditorOverlay.ToSharedRef());
        }


        this->ChildSlot
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    WeightWidget.ToSharedRef()
                ]

                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SNullWidget::NullWidget
                ]

                + SHorizontalBox::Slot()
                [
                    CloseButtonWidget.ToSharedRef()
                ]

            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SBorder)
				.BorderImage(FDungeonArchitectStyle::Get().GetBrush("DungeonArchitect.RoundDarkBorder"))
				.BorderBackgroundColor(this, &SGrammarRuleGraph::GetBorderColor)
                [
                    GraphEditorOverlay.ToSharedRef()
                ]
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SSpacer)
                .Size(FVector2D(10, 10))
            ]
        ];
    }

    else {
        GraphEditorHost = SNew(SBox);

        TSharedPtr<SWidget> Content = GraphEditorHost;
        if (DropHandler.IsValid()) {
            Content = WrapDropTarget(Content.ToSharedRef());
        }

        this->ChildSlot
        [
            Content.ToSharedRef()
        ];
    }

    // Set the default graph
    SetGraph(InGraph);
}

void SGrammarRuleGraph::OnGraphChangedCallback(const struct FEdGraphEditAction& InAction) {
    NotifyGraphChanged();
}

void SGrammarRuleGraph::NotifyGraphChanged() {
    if (OnGraphChanged.IsBound()) {
        OnGraphChanged.Execute(Graph);
    }
}

namespace {
    template <typename T>
    T ExtractDragOperationData(TSharedPtr<FDragDropOperation> Operation) {
        if (Operation->IsOfType<FEditableListItemDragDropOp<T>>()) {
            TSharedPtr<FEditableListItemDragDropOp<T>> ListDropOp =
                StaticCastSharedPtr<FEditableListItemDragDropOp<T>>(Operation);
            if (ListDropOp->bAllowDropOnGraph) {
                return ListDropOp->ItemData->Item;
            }
        }
        return nullptr;
    }

    /*
    FVector2D GetDropGridLocation() const
    {
        FVector2D PanelCoord = AssetDropTarget->GetPanelCoordDropPosition();
        FVector2D ViewLocation = FVector2D::ZeroVector;
        float ZoomAmount = 1.0f;
        GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);
        FVector2D GridLocation = PanelCoord / ZoomAmount + ViewLocation;

        return GridLocation;
    }
    */
}

FReply SGrammarRuleGraph::OnNodeTypeDrop(TSharedPtr<FDragDropOperation> Operation) {
    if (DropHandler.IsValid() && GraphEditor.IsValid() && Graph.IsValid()) {
        FVector2D DropPosition = DropTarget.IsValid() ? DropTarget->GetPanelCoordDropPosition() : FVector2D::ZeroVector;
        DropHandler->OnNodeTypeDrop(Operation, DropPosition, GraphEditor, Graph);
    }
    return FReply::Unhandled();
}

bool SGrammarRuleGraph::OnNodeTypeAllowDrop(TSharedPtr<FDragDropOperation> Operation) {
    if (DropHandler.IsValid() && GraphEditor.IsValid() && Graph.IsValid()) {
        return DropHandler->OnNodeTypeAllowDrop(Operation, GraphEditor, Graph);
    }
    return false;
}

bool SGrammarRuleGraph::OnIsNodeTypeDropRecognized(TSharedPtr<FDragDropOperation> Operation) {
    if (DropHandler.IsValid() && GraphEditor.IsValid() && Graph.IsValid()) {
        return DropHandler->OnIsNodeTypeDropRecognized(Operation, GraphEditor, Graph);
    }
    return false;
}

TSharedPtr<SWidget> SGrammarRuleGraph::WrapDropTarget(TSharedRef<SWidget> Widget) {
    DropTarget = SNew(SGrammarRuleGraphDropTarget)
		.OnDrop_Raw(this, &SGrammarRuleGraph::OnNodeTypeDrop)
		.OnAllowDrop_Raw(this, &SGrammarRuleGraph::OnNodeTypeAllowDrop)
		.OnIsRecognized_Raw(this, &SGrammarRuleGraph::OnIsNodeTypeDropRecognized)
		.BackgroundColor(FLinearColor(1, 1, 1, 0.125f))
		.BackgroundColorHover(FLinearColor(1, 1, 1, 0.25f))
		.Content()
    [
        Widget
    ];

    return DropTarget;
}

void SGrammarRuleGraph::SetGraph(UEdGraph_Grammar* InGraph) {
    // Remove the old graph change handler
    if (Graph.IsValid()) {
        Graph->RemoveOnGraphChangedHandler(GraphChangedHandler);
    }

    Graph = InGraph;

    if (InGraph) {
        // Recreate the graph editor widget with the new graph
        CreateGraphEditorWidget();
        GraphEditorHost->SetContent(GraphEditor.ToSharedRef());
        GraphChangedHandler = Graph->AddOnGraphChangedHandler(
            FOnGraphChanged::FDelegate::CreateRaw(this, &SGrammarRuleGraph::OnGraphChangedCallback));
    }
    else {
        GraphEditor = nullptr;
        GraphHandler = nullptr;
        GraphEditorHost->SetContent(SNullWidget::NullWidget);
    }
}

void SGrammarRuleGraph::ZoomToFit() {
    GraphEditor->ZoomToFit(false);
}

bool GetLinkedNodeFromSelection(TSharedPtr<SGraphEditor> GraphEditor, UEdGraphNode_GrammarNode** OutNodeSource,
                                UEdGraphNode_GrammarNode** OutNodeDest) {
    TArray<UEdGraphNode_GrammarNode*> SelectedNodes;
    for (UObject* SelectedObject : GraphEditor->GetSelectedNodes()) {
        UEdGraphNode_GrammarNode* GrammarNode = Cast<UEdGraphNode_GrammarNode>(SelectedObject);
        if (GrammarNode) {
            SelectedNodes.Add(GrammarNode);
        }
    }
    if (SelectedNodes.Num() != 2) {
        return false;
    }

    UEdGraphNode_GrammarNode* A = SelectedNodes[0];
    UEdGraphNode_GrammarNode* B = SelectedNodes[1];

    // Check if there's a link from A -> B
    if (A->GetOutputPin()->LinkedTo.Contains(B->GetInputPin())) {
        *OutNodeSource = A;
        *OutNodeDest = B;
        return true;
    }
    if (B->GetOutputPin()->LinkedTo.Contains(A->GetInputPin())) {
        *OutNodeSource = B;
        *OutNodeDest = A;
        return true;
    }
    *OutNodeSource = nullptr;
    *OutNodeDest = nullptr;
    return false;
}

FReply SGrammarRuleGraph::ConvertLinkToNonDependent() {
    UEdGraphNode_GrammarNode* SourceNode = nullptr;
    UEdGraphNode_GrammarNode* DestNode = nullptr;
    if (GetLinkedNodeFromSelection(GraphEditor, &SourceNode, &DestNode)) {
        SourceNode->DependentNodes.Remove(DestNode->NodeGuid);
        NotifyGraphChanged();
    }
    else {
        // Show usage dialog
        ShowUsageHelpDialog();
    }

    return FReply::Handled();
}

FReply SGrammarRuleGraph::ConvertLinkToDependent() {
    UEdGraphNode_GrammarNode* SourceNode = nullptr;
    UEdGraphNode_GrammarNode* DestNode = nullptr;
    if (GetLinkedNodeFromSelection(GraphEditor, &SourceNode, &DestNode)) {
        SourceNode->DependentNodes.Add(DestNode->NodeGuid);
        NotifyGraphChanged();
    }
    else {
        // Show usage dialog
        ShowUsageHelpDialog();
    }

    return FReply::Handled();
}

void SGrammarRuleGraph::Flash() {
    Flasher.Flash();
}

void SGrammarRuleGraph::Focus() {
    FSlateApplication::Get().SetAllUserFocus(GraphEditor);
}

void SGrammarRuleGraph::JumpToNode(UEdGraphNode* Node) {
    if (GraphEditor.IsValid()) {
        GraphEditor->JumpToNode(Node);
    }
}

FReply SGrammarRuleGraph::OnDeleteClicked() {
    return OnDelete.IsBound() ? OnDelete.Execute(Graph) : FReply::Handled();
}

void SGrammarRuleGraph::CreateGraphEditorWidget() {
    // Create the appearance info
    FGraphAppearanceInfo AppearanceInfo;
    AppearanceInfo.CornerText = FText::FromString(Title);
    GraphHandler = MakeShareable(new FGrammarRuleGraphHandler);
    GraphHandler->Bind();
    GraphHandler->SetPropertyEditor(PropertyEditor);

    GraphEditor = SNew(SGraphEditor)
		.AdditionalCommands(GraphHandler->GraphEditorCommands)
		.Appearance(AppearanceInfo)
		.GraphToEdit(Graph.Get())
		.IsEditable(bIsEditable)
		.ShowGraphStateOverlay(false)
		.GraphEvents(GraphHandler->GraphEvents);

    GraphHandler->SetGraphEditor(GraphEditor);
}

void SGrammarRuleGraph::ShowUsageHelpDialog() {
    FText MessageTitle = LOCTEXT("LinkConvertHelpTitle", "Link Conversion");
    FMessageDialog::Open(EAppMsgType::Ok, EAppReturnType::Ok,
                         LOCTEXT("LinkConvertHelpMessage",
                                 "Please select two connected nodes first, to convert an existing link"),
                         &MessageTitle);
}

FText SGrammarRuleGraph::GetGraphWeightText() const {
    if (!Graph.IsValid()) {
        return FText();
    }

    UGrammarRuleScript* OwnerScript = Cast<UGrammarRuleScript>(Graph->GetOuter());
    float Weight = OwnerScript ? OwnerScript->Weight : 0.0f;
    return FText::FromString(FString::SanitizeFloat(Weight, 2));
}

void SGrammarRuleGraph::OnGraphWeightTextCommit(const FText& InText, ETextCommit::Type InCommitType) {
    if (Graph.IsValid()) {
        float Weight = FCString::Atof(*InText.ToString());
        UGrammarRuleScript* OwnerScript = Cast<UGrammarRuleScript>(Graph->GetOuter());
        if (OwnerScript) {
            OwnerScript->Weight = Weight;
        }
    }
}

FSlateColor SGrammarRuleGraph::GetBorderColor() const {
    if (Flasher.IsFlashing()) {
        return Flasher.GetFlashCurveColor();
    }
    return FSlateColor(FColor(32, 32, 32));
}

////////////////////////////////// FFlowGraphHandler //////////////////////////////////////

void FGrammarRuleGraphHandler::Bind() {
    GraphEditorCommands = MakeShareable(new FUICommandList);
    GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
                                   FExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::SelectAllNodes),
                                   FCanExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::CanSelectAllNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
                                   FExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::DeleteSelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::CanDeleteNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
                                   FExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::CopySelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::CanCopyNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
                                   FExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::PasteNodes),
                                   FCanExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::CanPasteNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
                                   FExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::CutSelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::CanCutNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
                                   FExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::DuplicateNodes),
                                   FCanExecuteAction::CreateSP(this, &FGrammarRuleGraphHandler::CanDuplicateNodes)
    );

    GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(
        this, &FGrammarRuleGraphHandler::HandleSelectedNodesChanged);
    GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(
        this, &FGrammarRuleGraphHandler::HandleNodeDoubleClicked);
}

void FGrammarRuleGraphHandler::SetGraphEditor(TSharedPtr<SGraphEditor> InGraphEditor) {
    this->GraphEditor = InGraphEditor;
}

void FGrammarRuleGraphHandler::SetPropertyEditor(TWeakPtr<IDetailsView> InPropertyEditor) {
    PropertyEditor = InPropertyEditor;
}

void FGrammarRuleGraphHandler::SelectAllNodes() {
    if (!GraphEditor.IsValid()) return;
    GraphEditor->SelectAllNodes();
}

bool FGrammarRuleGraphHandler::CanSelectAllNodes() const {
    return GraphEditor.IsValid();
}

void FGrammarRuleGraphHandler::DeleteSelectedNodes() {
    if (!GraphEditor.IsValid()) return;
    TArray<UEdGraphNode*> NodesToDelete;
    const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();

    for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt) {
        NodesToDelete.Add(CastChecked<UEdGraphNode>(*NodeIt));
    }

    DeleteNodes(NodesToDelete);

    if (NodesToDelete.Num() > 0) {
        GraphEditor->GetCurrentGraph()->NotifyGraphChanged();
    }
}

bool FGrammarRuleGraphHandler::CanDeleteNode(class UEdGraphNode* Node) {
    if (!GraphEditor.IsValid()) return false;
    if (Node->IsA<UEdGraphNode_ExecEntryNode>()) {
        return false;
    }
    return true;
}

void FGrammarRuleGraphHandler::DeleteNodes(const TArray<class UEdGraphNode*>& NodesToDelete) {
    if (!GraphEditor.IsValid()) return;
    if (NodesToDelete.Num() > 0) {

        for (int32 Index = 0; Index < NodesToDelete.Num(); ++Index) {
            if (!CanDeleteNode(NodesToDelete[Index])) {
                continue;
            }

            // Break all node links first so that we don't update the material before deleting
            NodesToDelete[Index]->BreakAllNodeLinks();

            FBlueprintEditorUtils::RemoveNode(nullptr, NodesToDelete[Index], true);

            // TODO: Process deletion in the data model
        }
    }
}

void FGrammarRuleGraphHandler::DeleteSelectedDuplicatableNodes() {
    if (!GraphEditor.IsValid()) return;
    // Cache off the old selection
    const FGraphPanelSelectionSet OldSelectedNodes = GraphEditor->GetSelectedNodes();

    // Clear the selection and only select the nodes that can be duplicated
    FGraphPanelSelectionSet RemainingNodes;
    GraphEditor->ClearSelectionSet();

    for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter) {
        UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
        if ((Node != nullptr) && Node->CanDuplicateNode()) {
            GraphEditor->SetNodeSelection(Node, true);
        }
        else {
            RemainingNodes.Add(Node);
        }
    }

    // Delete the duplicatable nodes
    DeleteSelectedNodes();

    // Reselect whatever is left from the original selection after the deletion
    GraphEditor->ClearSelectionSet();

    for (FGraphPanelSelectionSet::TConstIterator SelectedIter(RemainingNodes); SelectedIter; ++SelectedIter) {
        if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter)) {
            GraphEditor->SetNodeSelection(Node, true);
        }
    }
}

bool FGrammarRuleGraphHandler::CanDeleteNodes() const {
    if (!GraphEditor.IsValid()) return false;
    return true;
}

void FGrammarRuleGraphHandler::CopySelectedNodes() {
    if (!GraphEditor.IsValid()) return;

    // Export the selected nodes and place the text on the clipboard
    const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();

    FString ExportedText;

    for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter) {
        if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter)) {
            Node->PrepareForCopying();
        }
    }

    FEdGraphUtilities::ExportNodesToText(SelectedNodes, /*out*/ ExportedText);
    FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool FGrammarRuleGraphHandler::CanCopyNodes() const {
    if (!GraphEditor.IsValid()) return false;

    // If any of the nodes can be duplicated then we should allow copying
    const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
    for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter) {
        UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
        if ((Node != nullptr) && Node->CanDuplicateNode()) {
            return true;
        }
    }
    return false;
}

void FGrammarRuleGraphHandler::PasteNodes() {
    if (!GraphEditor.IsValid()) return;

    PasteNodesHere(GraphEditor->GetPasteLocation());
}

bool FGrammarRuleGraphHandler::CanPasteNodes() const {
    if (!GraphEditor.IsValid()) return false;

    FString ClipboardContent;
    FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

    return FEdGraphUtilities::CanImportNodesFromText(GraphEditor->GetCurrentGraph(), ClipboardContent);
}

void FGrammarRuleGraphHandler::PasteNodesHere(const FVector2D& Location) {
    if (!GraphEditor.IsValid()) return;

    // Undo/Redo support
    const FScopedTransaction Transaction(NSLOCTEXT("DungeonArchitect", "DungeonEditorPaste", "Dungeon Editor: Paste"));
    // TODO: Notify the data model of modification
    //Material->MaterialGraph->Modify();
    //Material->Modify();

    // Clear the selection set (newly pasted stuff will be selected)
    GraphEditor->ClearSelectionSet();

    // Grab the text to paste from the clipboard.
    FString TextToImport;
    FPlatformApplicationMisc::ClipboardPaste(TextToImport);

    // Import the nodes
    TSet<UEdGraphNode*> PastedNodes;
    FEdGraphUtilities::ImportNodesFromText(GraphEditor->GetCurrentGraph(), TextToImport, /*out*/ PastedNodes);

    //Average position of nodes so we can move them while still maintaining relative distances to each other
    FVector2D AvgNodePosition(0.0f, 0.0f);

    for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It) {
        UEdGraphNode* Node = *It;
        AvgNodePosition.X += Node->NodePosX;
        AvgNodePosition.Y += Node->NodePosY;
    }

    if (PastedNodes.Num() > 0) {
        float InvNumNodes = 1.0f / static_cast<float>(PastedNodes.Num());
        AvgNodePosition.X *= InvNumNodes;
        AvgNodePosition.Y *= InvNumNodes;
    }

    for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It) {
        UEdGraphNode* Node = *It;

        // Select the newly pasted stuff
        GraphEditor->SetNodeSelection(Node, true);

        Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
        Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

        Node->SnapToGrid(SNodePanel::GetSnapGridSize());

        // Give new node a different Guid from the old one
        Node->CreateNewGuid();
        Node->PostPlacedNewNode();
        Node->AllocateDefaultPins();
    }

    // TODO: Implement
    //UpdatePropAfterGraphChange();

    // Update UI
    GraphEditor->NotifyGraphChanged();
}

void FGrammarRuleGraphHandler::CutSelectedNodes() {
    if (!GraphEditor.IsValid()) return;

    CopySelectedNodes();
    // Cut should only delete nodes that can be duplicated
    DeleteSelectedDuplicatableNodes();
}

bool FGrammarRuleGraphHandler::CanCutNodes() const {
    if (!GraphEditor.IsValid()) return false;

    return CanCopyNodes() && CanDeleteNodes();
}

void FGrammarRuleGraphHandler::DuplicateNodes() {
    if (!GraphEditor.IsValid()) return;

    // Copy and paste current selection
    CopySelectedNodes();
    PasteNodes();
}

bool FGrammarRuleGraphHandler::CanDuplicateNodes() const {
    if (!GraphEditor.IsValid()) return false;

    return CanCopyNodes();
}

void FGrammarRuleGraphHandler::HandleSelectedNodesChanged(const TSet<class UObject*>& NewSelection) {
    if (PropertyEditor.IsValid()) {
        TSharedPtr<IDetailsView> DetailsView = PropertyEditor.Pin();
        DetailsView->SetObjects(NewSelection.Array());
    }

    OnNodeSelectionChanged.ExecuteIfBound(NewSelection);
}

void FGrammarRuleGraphHandler::HandleNodeDoubleClicked(class UEdGraphNode* Node) {
    OnNodeDoubleClicked.ExecuteIfBound(Node);
}

/////////////////////////////////// FNodeTypeGraphDropHandler /////////////////////////////////// 

void FNodeTypeGraphDropHandler::OnNodeTypeDrop(TSharedPtr<FDragDropOperation> Operation,
                                               const FVector2D& InDropPosition, TSharedPtr<SGraphEditor> GraphEditor,
                                               TWeakObjectPtr<UEdGraph> Graph) {
    UEdGraph_Grammar* GrammarGraph = Cast<UEdGraph_Grammar>(Graph.Get());
    UObject* DropData = ExtractDragOperationData<UObject*>(Operation);
    UGrammarNodeType* DroppedNodeType = Cast<UGrammarNodeType>(DropData);
    if (DroppedNodeType && GrammarGraph) {
        FVector2D PanelCoord = InDropPosition;
        FVector2D ViewLocation = FVector2D::ZeroVector;
        float ZoomAmount = 1.0f;
        GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);
        FVector2D GridLocation = PanelCoord / ZoomAmount + ViewLocation;

        // Spawn an grammar node in this location
        UEdGraphNode_GrammarNode* Node = GrammarGraph->AddNewNode(DroppedNodeType);
        const float NodeOffset = 24;
        if (Node) {
            Node->NodePosX = GridLocation.X - NodeOffset;
            Node->NodePosY = GridLocation.Y - NodeOffset;
            Node->AssignNextAvailableNodeIndex();

            GrammarGraph->NotifyGraphChanged();
        }
    }
}

bool FNodeTypeGraphDropHandler::OnNodeTypeAllowDrop(TSharedPtr<FDragDropOperation> Operation,
                                                    TSharedPtr<SGraphEditor> GraphEditor,
                                                    TWeakObjectPtr<UEdGraph> Graph) {
    UObject* DropData = ExtractDragOperationData<UObject*>(Operation);
    UGrammarNodeType* DroppedNodeType = Cast<UGrammarNodeType>(DropData);
    return DroppedNodeType != nullptr;
}

bool FNodeTypeGraphDropHandler::OnIsNodeTypeDropRecognized(TSharedPtr<FDragDropOperation> Operation,
                                                           TSharedPtr<SGraphEditor> GraphEditor,
                                                           TWeakObjectPtr<UEdGraph> Graph) {
    UObject* DropData = ExtractDragOperationData<UObject*>(Operation);
    UGrammarNodeType* DroppedNodeType = Cast<UGrammarNodeType>(DropData);
    return DroppedNodeType != nullptr;
}

/////////////////////////////////// FProductionRuleGraphDropHandler /////////////////////////////////// 

void FProductionRuleGraphDropHandler::OnNodeTypeDrop(TSharedPtr<FDragDropOperation> Operation,
                                                     const FVector2D& InDropPosition,
                                                     TSharedPtr<SGraphEditor> GraphEditor,
                                                     TWeakObjectPtr<UEdGraph> Graph) {
    UEdGraph_FlowExec* ExecutionGraph = Cast<UEdGraph_FlowExec>(Graph.Get());

    UObject* DropData = ExtractDragOperationData<UObject*>(Operation);
    UGraphGrammarProduction* DroppedRule = Cast<UGraphGrammarProduction>(DropData);
    if (DroppedRule && ExecutionGraph) {
        FVector2D PanelCoord = InDropPosition;
        FVector2D ViewLocation = FVector2D::ZeroVector;
        float ZoomAmount = 1.0f;
        GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);
        FVector2D GridLocation = PanelCoord / ZoomAmount + ViewLocation;


        UEdGraphNode_ExecRuleNode* RuleNode = ExecutionGraph->AddNewNode(DroppedRule);
        const float NodeOffset = 24;
        if (RuleNode) {
            RuleNode->NodePosX = GridLocation.X - NodeOffset;
            RuleNode->NodePosY = GridLocation.Y - NodeOffset;

            ExecutionGraph->NotifyGraphChanged();
        }
    }
}

bool FProductionRuleGraphDropHandler::OnNodeTypeAllowDrop(TSharedPtr<FDragDropOperation> Operation,
                                                          TSharedPtr<SGraphEditor> GraphEditor,
                                                          TWeakObjectPtr<UEdGraph> Graph) {
    UObject* DropData = ExtractDragOperationData<UObject*>(Operation);
    UGraphGrammarProduction* DroppedRule = Cast<UGraphGrammarProduction>(DropData);
    return DroppedRule != nullptr;
}

bool FProductionRuleGraphDropHandler::OnIsNodeTypeDropRecognized(TSharedPtr<FDragDropOperation> Operation,
                                                                 TSharedPtr<SGraphEditor> GraphEditor,
                                                                 TWeakObjectPtr<UEdGraph> Graph) {
    UObject* DropData = ExtractDragOperationData<UObject*>(Operation);
    UGraphGrammarProduction* DroppedRule = Cast<UGraphGrammarProduction>(DropData);
    return DroppedRule != nullptr;
}


#undef LOCTEXT_NAMESPACE

