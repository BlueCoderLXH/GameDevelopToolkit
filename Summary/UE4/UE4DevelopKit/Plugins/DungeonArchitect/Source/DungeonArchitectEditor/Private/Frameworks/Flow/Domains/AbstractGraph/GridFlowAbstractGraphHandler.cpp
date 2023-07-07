//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractGraphHandler.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodes.h"

#include "EdGraph/EdGraph.h"
#include "EdGraphUtilities.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDetailsView.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "SNodePanel.h"
#include "ScopedTransaction.h"

void FGridFlowAbstractGraphHandler::Bind() {
    GraphEditorCommands = MakeShareable(new FUICommandList);
    GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
                                   FExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::SelectAllNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::CanSelectAllNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
                                   FExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::DeleteSelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::CanDeleteNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
                                   FExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::CopySelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::CanCopyNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
                                   FExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::PasteNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::CanPasteNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
                                   FExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::CutSelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::CanCutNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
                                   FExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::DuplicateNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowAbstractGraphHandler::CanDuplicateNodes)
    );

    GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(
        this, &FGridFlowAbstractGraphHandler::HandleSelectedNodesChanged);
    GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(
        this, &FGridFlowAbstractGraphHandler::HandleNodeDoubleClicked);
}

void FGridFlowAbstractGraphHandler::SetGraphEditor(TSharedPtr<SGraphEditor> InGraphEditor) {
    this->GraphEditor = InGraphEditor;
}

void FGridFlowAbstractGraphHandler::SetPropertyEditor(TWeakPtr<IDetailsView> InPropertyEditor) {
    PropertyEditor = InPropertyEditor;
}

void FGridFlowAbstractGraphHandler::SelectAllNodes() {
    if (!GraphEditor.IsValid()) return;
    GraphEditor->SelectAllNodes();
}

bool FGridFlowAbstractGraphHandler::CanSelectAllNodes() const {
    return GraphEditor.IsValid();
}

void FGridFlowAbstractGraphHandler::DeleteSelectedNodes() {
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

bool FGridFlowAbstractGraphHandler::CanDeleteNode(class UEdGraphNode* Node) {
    if (!GraphEditor.IsValid()) return false;
    if (Node->IsA<UGridFlowAbstractEdGraphNode>()) {
        return false;
    }
    return true;
}

void FGridFlowAbstractGraphHandler::DeleteNodes(const TArray<class UEdGraphNode*>& NodesToDelete) {
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

void FGridFlowAbstractGraphHandler::DeleteSelectedDuplicatableNodes() {
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

bool FGridFlowAbstractGraphHandler::CanDeleteNodes() const {
    if (!GraphEditor.IsValid()) return false;
    return true;
}

void FGridFlowAbstractGraphHandler::CopySelectedNodes() {
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

bool FGridFlowAbstractGraphHandler::CanCopyNodes() const {
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

void FGridFlowAbstractGraphHandler::PasteNodes() {
    if (!GraphEditor.IsValid()) return;

    PasteNodesHere(GraphEditor->GetPasteLocation());
}

bool FGridFlowAbstractGraphHandler::CanPasteNodes() const {
    if (!GraphEditor.IsValid()) return false;

    FString ClipboardContent;
    FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

    return FEdGraphUtilities::CanImportNodesFromText(GraphEditor->GetCurrentGraph(), ClipboardContent);
}

void FGridFlowAbstractGraphHandler::PasteNodesHere(const FVector2D& Location) {
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

void FGridFlowAbstractGraphHandler::CutSelectedNodes() {
    if (!GraphEditor.IsValid()) return;

    CopySelectedNodes();
    // Cut should only delete nodes that can be duplicated
    DeleteSelectedDuplicatableNodes();
}

bool FGridFlowAbstractGraphHandler::CanCutNodes() const {
    if (!GraphEditor.IsValid()) return false;

    return CanCopyNodes() && CanDeleteNodes();
}

void FGridFlowAbstractGraphHandler::DuplicateNodes() {
    if (!GraphEditor.IsValid()) return;

    // Copy and paste current selection
    CopySelectedNodes();
    PasteNodes();
}

bool FGridFlowAbstractGraphHandler::CanDuplicateNodes() const {
    if (!GraphEditor.IsValid()) return false;

    return CanCopyNodes();
}

void FGridFlowAbstractGraphHandler::HandleSelectedNodesChanged(const TSet<class UObject*>& NewSelection) {
    if (PropertyEditor.IsValid()) {
        TSharedPtr<IDetailsView> DetailsView = PropertyEditor.Pin();
        DetailsView->SetObjects(NewSelection.Array());
    }

    OnNodeSelectionChanged.ExecuteIfBound(NewSelection);
}

void FGridFlowAbstractGraphHandler::HandleNodeDoubleClicked(class UEdGraphNode* Node) {
    OnNodeDoubleClicked.ExecuteIfBound(Node);
}

