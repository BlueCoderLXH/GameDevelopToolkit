//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/Editor/SGrammarRuleGraph.h"
#include "Frameworks/GraphGrammar/GraphGrammarProcessor.h"

#include "IDetailsView.h"

class USnapMapAsset;
class UGraphGrammar;
class UEdGraph_Grammar;
class FGrammarRuleGraphHandler;
class SGrammarErrorList;
class SGrammarEditor;
class SGraphEditor;
class FProductionRuleGraphDropHandler;
class USnapMapEditor_GraphDesignAppModeSettings;

class FSnapMapExecGraphEditorHandler {
public:
    FSnapMapExecGraphEditorHandler();
    void Initialize(UEdGraph_Grammar* InResultGraph, UGraphGrammar* InGrammarRules,
                    TWeakPtr<IDetailsView> InPropertyEditor, USnapMapEditor_GraphDesignAppModeSettings* InSettings);
    void CreateGraphEditor();
    TSharedRef<SWidget> CreateToolWidget();
    TSharedRef<SWidget> GetContent() const;

    FText Seed_GetValue() const;
    bool Seed_Verify(const FText& NewText, FText& OutErrorMessage) const;
    void Seed_Commit(const FText& InText, ETextCommit::Type CommitInfo);

    FReply OnNodeTypeDrop(TSharedPtr<FDragDropOperation> Operation) const;
    bool OnNodeTypeAllowDrop(TSharedPtr<FDragDropOperation> Operation) const;
    bool OnIsNodeTypeDropRecognized(TSharedPtr<FDragDropOperation> Operation) const;

    void ResetResultGraph();
    void ExecuteResultGraph();
    void PerformLayout();

private:
    FReply OnClick_ExecuteResultGraph();
    FReply OnClick_RandomizeSeed();
    void HandleStateChange_RandomizeOnBuild(ECheckBoxState InCheckboxState);
    ECheckBoxState GetState_RandomizeOnBuild() const;

    
    int32 GetSeed() const;
    void SetSeed(int32 InSeed);
    void RandomizeSeed();
    
private:
    TWeakObjectPtr<UGraphGrammar> GrammarRules;
    TWeakObjectPtr<UEdGraph_Grammar> ResultGraph;
    TWeakPtr<IDetailsView> PropertyEditor;

    FGraphGrammarProcessor GraphGrammarProcessor;
    TSharedPtr<SGraphEditor> ExecutionGraphWidget;
    TSharedPtr<class SFlowExecGraphDropTarget> ContentHost;
    TSharedPtr<FGrammarRuleGraphHandler> GraphHandler;
    TSharedPtr<FProductionRuleGraphDropHandler> DropHandler;
    TWeakObjectPtr<USnapMapEditor_GraphDesignAppModeSettings> Settings;
};


class FSnapMapResultGraphEditorHandler {
public:
    FSnapMapResultGraphEditorHandler();
    void Initialize(UEdGraph_Grammar* InResultGraph, bool bIsEditable = true);
    void ZoomToFit();

    TSharedRef<SGrammarRuleGraph> GetGraphWidget() const;

private:
    TWeakObjectPtr<UEdGraph_Grammar> ResultGraph;
    TSharedPtr<SGrammarRuleGraph> ResultGraphWidget;
};

