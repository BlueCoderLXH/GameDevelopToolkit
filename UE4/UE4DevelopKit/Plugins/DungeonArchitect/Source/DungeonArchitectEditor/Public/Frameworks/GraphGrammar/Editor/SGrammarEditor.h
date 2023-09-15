//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/Editor/SEditableListView.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"

#include "Widgets/SCompoundWidget.h"

class IDetailsView;
class SGrammarRuleEditor;
class UEdGraph_Grammar;

class SGrammarEditor : public SCompoundWidget {
public:
    DECLARE_DELEGATE(FOnGrammarStateChanged)

public:
    SLATE_BEGIN_ARGS(SGrammarEditor)    : _OnGrammarStateChanged() {}
        SLATE_ARGUMENT(TWeakPtr<IDetailsView>, PropertyEditor)
        SLATE_EVENT(FOnGrammarStateChanged, OnGrammarStateChanged)
    SLATE_END_ARGS()

public:
    /** SCompoundWidget functions */
    void Construct(const FArguments& InArgs, TWeakObjectPtr<UGraphGrammar> InGrammar);
    virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;

    ////////// Focus Actions //////////
    void FocusOnNodeType(UGrammarNodeType* NodeType);
    void FocusOnRule(UGraphGrammarProduction* Rule);
    void FocusOnGraph(UEdGraph_Grammar* Graph);
    void FocusOnGraphNode(UEdGraphNode* Node);
    void FlashNodeTypePanel();
    void FlashRulesPanel();
    ////////// End of Focus Actions //////////


private:

    ////////// RuleListView Handlers //////////
    void OnRuleSelectionChanged(UGraphGrammarProduction* Item, ESelectInfo::Type SelectInfo);
    FText GetRuleListRowText(UGraphGrammarProduction* InItem) const;
    const TArray<UGraphGrammarProduction*>* GetRuleList();
    void OnRuleDelete(UGraphGrammarProduction* InItem);
    void OnRuleAdd();
    void OnRuleReordered(UGraphGrammarProduction* Source, UGraphGrammarProduction* Dest);
    ////////// End of RuleListView Handlers //////////

    ////////// NodeTypeListView Handlers //////////
    void OnNodeTypeSelectionChanged(UGrammarNodeType* Item, ESelectInfo::Type SelectInfo);
    FText GetNodeTypeName(UGrammarNodeType* InItem) const;
    FText GetNodeTypeDescription(UGrammarNodeType* InItem) const;
    const TArray<UGrammarNodeType*>* GetNodeTypeList();
    void OnNodeTypeDelete(UGrammarNodeType* InItem);
    void OnNodeTypeAdd();
    void OnNodeTypeReordered(UGrammarNodeType* Source, UGrammarNodeType* Dest);
    TSharedPtr<SWidget> CreateNodeListItem(UGrammarNodeType* InItem);
    ////////// End of RuleListView Handlers //////////

    // Callbacks
    void OnRuleGraphChanged(TWeakObjectPtr<UGraphGrammarProduction> Rule, TWeakObjectPtr<UEdGraph_Grammar> Graph);
    void NotifyGrammarStateChanged();

private:
    TWeakObjectPtr<UGraphGrammar> Grammar;

    TWeakPtr<IDetailsView> PropertyEditor;
    TSharedPtr<SGrammarRuleEditor> RuleEditor;

    TSharedPtr<SEditableListView<UGraphGrammarProduction*>> RuleListView;
    TSharedPtr<SEditableListView<UGrammarNodeType*>> NodeTypeListView;

    bool bRequestGrammarStateChanged;
    FOnGrammarStateChanged OnGrammarStateChanged;
};

