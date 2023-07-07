//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GraphEditor.h"

class IDetailsView;
class UGridFlowExecEdGraph;

DECLARE_DELEGATE_RetVal_OneParam(FReply, FExecGraphDelegate, TWeakObjectPtr<UGridFlowExecEdGraph>);
DECLARE_DELEGATE_OneParam(FExecGraphChangedDelegate, TWeakObjectPtr<UGridFlowExecEdGraph>);
DECLARE_DELEGATE_OneParam(FExecGraphNodeDelegate, UEdGraphNode*);
DECLARE_DELEGATE_OneParam(FExecGraphSelectedNodeChangedDelegate, const TSet<class UObject*>&);

class FGridFlowExecGraphHandler : public TSharedFromThis<FGridFlowExecGraphHandler> {
public:
    void Bind();
    void SetGraphEditor(TSharedPtr<SGraphEditor> InGraphEditor);
    void SetPropertyEditor(TWeakPtr<IDetailsView> InPropertyEditor);

private:

    /** Select every node in the graph */
    void SelectAllNodes();
    /** Whether we can select every node */
    bool CanSelectAllNodes() const;

    /** Deletes all the selected nodes */
    void DeleteSelectedNodes();

    bool CanDeleteNode(class UEdGraphNode* Node);

    /** Delete an array of Material Graph Nodes and their corresponding expressions/comments */
    void DeleteNodes(const TArray<class UEdGraphNode*>& NodesToDelete);

    /** Delete only the currently selected nodes that can be duplicated */
    void DeleteSelectedDuplicatableNodes();

    /** Whether we are able to delete the currently selected nodes */
    bool CanDeleteNodes() const;

    /** Copy the currently selected nodes */
    void CopySelectedNodes();
    /** Whether we are able to copy the currently selected nodes */
    bool CanCopyNodes() const;

    /** Paste the contents of the clipboard */
    void PasteNodes();
    bool CanPasteNodes() const;
    void PasteNodesHere(const FVector2D& Location);

    /** Cut the currently selected nodes */
    void CutSelectedNodes();
    /** Whether we are able to cut the currently selected nodes */
    bool CanCutNodes() const;

    /** Duplicate the currently selected nodes */
    void DuplicateNodes();
    /** Whether we are able to duplicate the currently selected nodes */
    bool CanDuplicateNodes() const;

    /** Called when the selection changes in the GraphEditor */
    void HandleSelectedNodesChanged(const TSet<class UObject*>& NewSelection);

    /** Called when a node is double clicked */
    void HandleNodeDoubleClicked(class UEdGraphNode* Node);

public:
    SGraphEditor::FGraphEditorEvents GraphEvents;
    TSharedPtr<FUICommandList> GraphEditorCommands;

    FExecGraphNodeDelegate OnNodeDoubleClicked;
    FExecGraphSelectedNodeChangedDelegate OnNodeSelectionChanged;

private:
    TSharedPtr<SGraphEditor> GraphEditor;
    TWeakPtr<IDetailsView> PropertyEditor;
};

