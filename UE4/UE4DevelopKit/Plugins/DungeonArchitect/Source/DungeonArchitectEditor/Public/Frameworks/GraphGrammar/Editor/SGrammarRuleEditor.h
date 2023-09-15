//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;
class SScrollBox;
class UGraphGrammarProduction;
class SGrammarRuleGraph;
class UEdGraph_Grammar;
class IDetailsView;

DECLARE_DELEGATE_TwoParams(FOnRuleGraphChanged, TWeakObjectPtr<UGraphGrammarProduction>,
                           TWeakObjectPtr<UEdGraph_Grammar>);

class SGrammarRuleEditor : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SGrammarRuleEditor)
            : _OnRuleGraphChanged() {}
        SLATE_ARGUMENT(TWeakObjectPtr<UGraphGrammarProduction>, Rule)
        SLATE_ARGUMENT(TWeakPtr<IDetailsView>, PropertyEditor)
        SLATE_EVENT(FOnRuleGraphChanged, OnRuleGraphChanged)
    SLATE_END_ARGS()

public:
    /** SCompoundWidget functions */
    void Construct(const FArguments& InArgs);
    void SetRule(TWeakObjectPtr<UGraphGrammarProduction> InRule);
    void FocusOnGraph(UEdGraph_Grammar* Graph);
    void FocusOnGraphNode(UEdGraphNode* GrammarNode);

private:
    FText GetRuleName() const;
    void AddRHSWidget(UEdGraph_Grammar* RHSGraph);
    void DeleteRHSGraph(TWeakObjectPtr<UEdGraph_Grammar> RHSGraph);

    FReply OnAddRHSRuleClicked();
    FReply OnDeleteRHSRuleClicked(TWeakObjectPtr<UEdGraph_Grammar> RHSGraph);
    TSharedPtr<SGrammarRuleGraph> FindGraphWidget(UEdGraph_Grammar* Graph);
    void OnGraphChangedCallback(TWeakObjectPtr<UEdGraph_Grammar> Graph);
    void NotifyRuleChanged(TWeakObjectPtr<UEdGraph_Grammar> Graph);

private:
    TWeakObjectPtr<UGraphGrammarProduction> Rule;
    TSharedPtr<SGrammarRuleGraph> LHSWidget;
    TSharedPtr<SVerticalBox> RHSWidgets;
    TSharedPtr<SScrollBox> ScrollBox;
    TWeakPtr<IDetailsView> PropertyEditor;
    FOnRuleGraphChanged OnRuleGraphChanged;
};

