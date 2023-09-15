//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/DungeonThemeGraphHandler.h"

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonActorTemplate.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarker.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMesh.h"

#include "EdGraphUtilities.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "SNodePanel.h"
#include "ScopedTransaction.h"

void FDungeonArchitectThemeGraphHandler::Initialize(TSharedPtr<SGraphEditor> InGraphEditor,
                                                    TSharedPtr<IDetailsView> InPropertyEditor,
                                                    UEdGraph* ThemeGraph, UObject* InDefaultPropertyObject) {
    GraphEditor = InGraphEditor;
    PropertyEditor = InPropertyEditor;
    UpdateGraph = ThemeGraph;
    DefaultPropertyObject = InDefaultPropertyObject;
}

void FDungeonArchitectThemeGraphHandler::Bind() {
    GraphEditorCommands = MakeShareable(new FUICommandList);

    GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
                                   FExecuteAction::CreateSP(this, &FDungeonArchitectThemeGraphHandler::SelectAllNodes),
                                   FCanExecuteAction::CreateSP(
                                       this, &FDungeonArchitectThemeGraphHandler::CanSelectAllNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
                                   FExecuteAction::CreateSP(
                                       this, &FDungeonArchitectThemeGraphHandler::DeleteSelectedNodes),
                                   FCanExecuteAction::CreateSP(
                                       this, &FDungeonArchitectThemeGraphHandler::CanDeleteNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
                                   FExecuteAction::CreateSP(
                                       this, &FDungeonArchitectThemeGraphHandler::CopySelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FDungeonArchitectThemeGraphHandler::CanCopyNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
                                   FExecuteAction::CreateSP(this, &FDungeonArchitectThemeGraphHandler::PasteNodes),
                                   FCanExecuteAction::CreateSP(this, &FDungeonArchitectThemeGraphHandler::CanPasteNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
                                   FExecuteAction::CreateSP(
                                       this, &FDungeonArchitectThemeGraphHandler::CutSelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FDungeonArchitectThemeGraphHandler::CanCutNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
                                   FExecuteAction::CreateSP(this, &FDungeonArchitectThemeGraphHandler::DuplicateNodes),
                                   FCanExecuteAction::CreateSP(
                                       this, &FDungeonArchitectThemeGraphHandler::CanDuplicateNodes)
    );

    GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(
        this, &FDungeonArchitectThemeGraphHandler::OnSelectedNodesChanged);
    GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(
        this, &FDungeonArchitectThemeGraphHandler::OnNodeDoubleClicked);
}


void FDungeonArchitectThemeGraphHandler::SelectAllNodes() {
    GraphEditor->SelectAllNodes();
}

bool FDungeonArchitectThemeGraphHandler::CanSelectAllNodes() const {
    return GraphEditor.IsValid();
}

void FDungeonArchitectThemeGraphHandler::DeleteSelectedNodes() {
    TArray<UEdGraphNode*> NodesToDelete;
    const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();

    for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt) {
        NodesToDelete.Add(CastChecked<UEdGraphNode>(*NodeIt));
    }

    DeleteNodes(NodesToDelete);
}

bool FDungeonArchitectThemeGraphHandler::CanDeleteNode(class UEdGraphNode* Node) {
    bool CanDelete = true;

    if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Node)) {
        CanDelete = MarkerNode->bUserDefined;
    }

    return CanDelete;
}

void FDungeonArchitectThemeGraphHandler::DeleteNodes(const TArray<class UEdGraphNode*>& NodesToDelete) {
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

void FDungeonArchitectThemeGraphHandler::DeleteSelectedDuplicatableNodes() {
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

bool FDungeonArchitectThemeGraphHandler::CanDeleteNodes() const {
    return true;
}

void FDungeonArchitectThemeGraphHandler::CopySelectedNodes() {
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

    // Make sure the owner remains the same for the copied node
    // TODO: Check MaterialEditor.cpp for reference

}

bool FDungeonArchitectThemeGraphHandler::CanCopyNodes() const {
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

void FDungeonArchitectThemeGraphHandler::PasteNodes() {

    PasteNodesHere(GraphEditor->GetPasteLocation());
}

void FDungeonArchitectThemeGraphHandler::PasteNodesHere(const FVector2D& Location) {
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
    if (!UpdateGraph) return;
    TSet<UEdGraphNode*> PastedNodes;
    FEdGraphUtilities::ImportNodesFromText(UpdateGraph, TextToImport, /*out*/ PastedNodes);

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
        if (UEdGraphNode_DungeonMesh* MeshNode = Cast<UEdGraphNode_DungeonMesh>(Node)) {
            // TODO: Handle
        }
        else if (UEdGraphNode_DungeonActorTemplate* ActorTemplateNode = Cast<UEdGraphNode_DungeonActorTemplate>(Node)) {
            // TODO: Handle
        }
        else if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Node)) {
            MarkerNode->bUserDefined = true;
        }

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

bool FDungeonArchitectThemeGraphHandler::CanPasteNodes() const {
    FString ClipboardContent;
    FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
    return FEdGraphUtilities::CanImportNodesFromText(UpdateGraph, ClipboardContent);
}

void FDungeonArchitectThemeGraphHandler::CutSelectedNodes() {
    CopySelectedNodes();
    // Cut should only delete nodes that can be duplicated
    DeleteSelectedDuplicatableNodes();
}

bool FDungeonArchitectThemeGraphHandler::CanCutNodes() const {
    return CanCopyNodes() && CanDeleteNodes();
}

void FDungeonArchitectThemeGraphHandler::DuplicateNodes() {
    // Copy and paste current selection
    CopySelectedNodes();
    PasteNodes();
}

bool FDungeonArchitectThemeGraphHandler::CanDuplicateNodes() const {
    return CanCopyNodes();
}


void FDungeonArchitectThemeGraphHandler::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection) {
    if (NewSelection.Num() > 0) {
        TArray<UObject*> SelectedObjects;
        for (UObject* Object : NewSelection) {
            SelectedObjects.Add(Object);
        }
        if (PropertyEditor.IsValid()) {
            PropertyEditor->SetObjects(SelectedObjects);
        }
    }
    else {
        if (PropertyEditor.IsValid()) {
            PropertyEditor->SetObject(DefaultPropertyObject, true);
        }
    }
}

void FDungeonArchitectThemeGraphHandler::OnNodeDoubleClicked(class UEdGraphNode* Node) {

}


void FDungeonArchitectThemeGraphHandler::AddReferencedObjects(FReferenceCollector& Collector) {
    Collector.AddReferencedObject(UpdateGraph);
    Collector.AddReferencedObject(DefaultPropertyObject);
}

