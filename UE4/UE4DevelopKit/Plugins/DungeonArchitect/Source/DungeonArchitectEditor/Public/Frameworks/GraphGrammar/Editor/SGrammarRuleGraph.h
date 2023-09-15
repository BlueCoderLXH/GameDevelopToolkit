//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Common/Utils/DungeonEditorUtils.h"

#include "Widgets/SCompoundWidget.h"

class SBox;
class IDetailsView;
class UEdGraph_Grammar;

DECLARE_DELEGATE_RetVal_OneParam(FReply, FRuleGraphDelegate, TWeakObjectPtr<UEdGraph_Grammar>);
DECLARE_DELEGATE_OneParam(FRuleGraphChangedDelegate, TWeakObjectPtr<UEdGraph_Grammar>);

class IGrammarGraphDropHandler {
public:
    virtual ~IGrammarGraphDropHandler() {
    }

    virtual void OnNodeTypeDrop(TSharedPtr<FDragDropOperation> Operation, const FVector2D& InDropPosition,
                                TSharedPtr<SGraphEditor> GraphEditor, TWeakObjectPtr<UEdGraph> Graph) = 0;
    virtual bool OnNodeTypeAllowDrop(TSharedPtr<FDragDropOperation> Operation, TSharedPtr<SGraphEditor> GraphEditor,
                                     TWeakObjectPtr<UEdGraph> Graph) = 0;
    virtual bool OnIsNodeTypeDropRecognized(TSharedPtr<FDragDropOperation> Operation,
                                            TSharedPtr<SGraphEditor> GraphEditor, TWeakObjectPtr<UEdGraph> Graph) = 0;
};

typedef TSharedPtr<IGrammarGraphDropHandler> IGrammarGraphDropHandlerPtr;

class FNodeTypeGraphDropHandler : public IGrammarGraphDropHandler {
public:
    virtual void OnNodeTypeDrop(TSharedPtr<FDragDropOperation> Operation, const FVector2D& InDropPosition,
                                TSharedPtr<SGraphEditor> GraphEditor, TWeakObjectPtr<UEdGraph> Graph) override;
    virtual bool OnNodeTypeAllowDrop(TSharedPtr<FDragDropOperation> Operation, TSharedPtr<SGraphEditor> GraphEditor,
                                     TWeakObjectPtr<UEdGraph> Graph) override;
    virtual bool OnIsNodeTypeDropRecognized(TSharedPtr<FDragDropOperation> Operation, TSharedPtr<SGraphEditor> GraphEditor,
                                            TWeakObjectPtr<UEdGraph> Graph) override;
};

class FProductionRuleGraphDropHandler : public IGrammarGraphDropHandler {
public:
    virtual void OnNodeTypeDrop(TSharedPtr<FDragDropOperation> Operation, const FVector2D& InDropPosition,
                                TSharedPtr<SGraphEditor> GraphEditor, TWeakObjectPtr<UEdGraph> Graph) override;
    virtual bool OnNodeTypeAllowDrop(TSharedPtr<FDragDropOperation> Operation, TSharedPtr<SGraphEditor> GraphEditor,
                                     TWeakObjectPtr<UEdGraph> Graph) override;
    virtual bool OnIsNodeTypeDropRecognized(TSharedPtr<FDragDropOperation> Operation, TSharedPtr<SGraphEditor> GraphEditor,
                                            TWeakObjectPtr<UEdGraph> Graph) override;
};

class SGrammarRuleGraph : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SGrammarRuleGraph)
            : _Title("Graph")
              , _bShowCloseButton(true)
              , _bShowWeight(true)
              , _bFullScreen(false)
              , _bShowToolBar(false)
              , _BorderFlashColor(FLinearColor::Red)
              , _IsEditable(true)
              , _OnDelete()
              , _OnGraphChanged() {
        }

        SLATE_ARGUMENT(FString, Title)
        SLATE_ARGUMENT(bool, bShowCloseButton)
        SLATE_ARGUMENT(bool, bShowWeight)
        SLATE_ARGUMENT(bool, bFullScreen)
        SLATE_ARGUMENT(bool, bShowToolBar)
        SLATE_ARGUMENT(FLinearColor, BorderFlashColor)
        SLATE_ARGUMENT(TWeakPtr<IDetailsView>, PropertyEditor)
        SLATE_ARGUMENT(TSharedPtr<IGrammarGraphDropHandler>, DropHandler)
        SLATE_ARGUMENT(bool, IsEditable)
        SLATE_EVENT(FRuleGraphDelegate, OnDelete)
        SLATE_EVENT(FRuleGraphChangedDelegate, OnGraphChanged)
    SLATE_END_ARGS()

public:
    virtual ~SGrammarRuleGraph();

    /** SCompoundWidget functions */
    void Construct(const FArguments& InArgs, UEdGraph_Grammar* InGraph);
    void SetGraph(UEdGraph_Grammar* InGraph);
    void ZoomToFit();

    FReply ConvertLinkToNonDependent();
    FReply ConvertLinkToDependent();

    TWeakObjectPtr<UEdGraph_Grammar> GetGraph() const { return Graph; }
    void Flash();
    void Focus();
    void JumpToNode(UEdGraphNode* Node);
    TSharedPtr<class FGrammarRuleGraphHandler> GetGraphHandler() const { return GraphHandler; }

private:
    FReply OnDeleteClicked();
    void CreateGraphEditorWidget();
    void ShowUsageHelpDialog();
    FText GetGraphWeightText() const;
    void OnGraphWeightTextCommit(const FText&, ETextCommit::Type);

    FSlateColor GetBorderColor() const;
    void OnGraphChangedCallback(const struct FEdGraphEditAction& InAction);
    void NotifyGraphChanged();

    FReply OnNodeTypeDrop(TSharedPtr<FDragDropOperation> Operation);
    bool OnNodeTypeAllowDrop(TSharedPtr<FDragDropOperation> Operation);
    bool OnIsNodeTypeDropRecognized(TSharedPtr<FDragDropOperation> Operation);
    TSharedPtr<SWidget> WrapDropTarget(TSharedRef<SWidget> Widget);

private:
    FString Title;
    TWeakObjectPtr<UEdGraph_Grammar> Graph;
    TSharedPtr<SGraphEditor> GraphEditor;
    TSharedPtr<SBox> GraphEditorHost;
    TSharedPtr<class FGrammarRuleGraphHandler> GraphHandler;
    TWeakPtr<IDetailsView> PropertyEditor;
    FWidgetFlasher Flasher;

    FRuleGraphDelegate OnDelete;
    FRuleGraphChangedDelegate OnGraphChanged;

    FDelegateHandle GraphChangedHandler;

    bool bIsEditable = true;
    TSharedPtr<class SGrammarRuleGraphDropTarget> DropTarget;
    TSharedPtr<IGrammarGraphDropHandler> DropHandler;
};

class FUICommandList;
class SGraphEditor;

DECLARE_DELEGATE_OneParam(FRuleNodeGraphDelegate, UEdGraphNode*);
DECLARE_DELEGATE_OneParam(FRuleSelectedNodeChangedDelegate, const TSet<class UObject*>&);

class FGrammarRuleGraphHandler : public TSharedFromThis<FGrammarRuleGraphHandler> {
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

    FRuleNodeGraphDelegate OnNodeDoubleClicked;
    FRuleSelectedNodeChangedDelegate OnNodeSelectionChanged;

private:
    TSharedPtr<SGraphEditor> GraphEditor;
    TWeakPtr<IDetailsView> PropertyEditor;
};

