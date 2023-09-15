//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Editor/SGrammarEditor.h"

#include "Core/Editors/SnapMapEditor/SnapMapEditorUtils.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Core/Utils/DungeonGraphUtils.h"
#include "Frameworks/GraphGrammar/Editor/SEditableListView.h"
#include "Frameworks/GraphGrammar/Editor/SGrammarRuleEditor.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"

#include "IDetailsView.h"
#include "Misc/MessageDialog.h"
#include "Styling/ISlateStyle.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"

DEFINE_LOG_CATEGORY_STATIC(LogGrammarEditor, Log, All);

#define LOCTEXT_NAMESPACE "GrammarEditorWidget"

void SGrammarEditor::Construct(const FArguments& InArgs, TWeakObjectPtr<UGraphGrammar> InGrammar) {
    Grammar = InGrammar;
    PropertyEditor = InArgs._PropertyEditor;
    OnGrammarStateChanged = InArgs._OnGrammarStateChanged;
    bRequestGrammarStateChanged = false;

    RuleEditor = SNew(SGrammarRuleEditor)
		.PropertyEditor(PropertyEditor)
		.OnRuleGraphChanged_Raw(this, &SGrammarEditor::OnRuleGraphChanged);

    RuleListView = SNew(SEditableListView<UGraphGrammarProduction*>)
		.GetListSource(this, &SGrammarEditor::GetRuleList)
		.OnSelectionChanged(this, &SGrammarEditor::OnRuleSelectionChanged)
		.OnAddItem(this, &SGrammarEditor::OnRuleAdd)
		.OnDeleteItem(this, &SGrammarEditor::OnRuleDelete)
		.OnReorderItem(this, &SGrammarEditor::OnRuleReordered)
		.GetItemText(this, &SGrammarEditor::GetRuleListRowText)
		.IconBrush(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.GraphIcon"))
		.AllowDropOnGraph(true)
		.Title("Rules");

    NodeTypeListView = SNew(SEditableListView<UGrammarNodeType*>)
		.GetListSource(this, &SGrammarEditor::GetNodeTypeList)
		.OnSelectionChanged(this, &SGrammarEditor::OnNodeTypeSelectionChanged)
		.OnAddItem(this, &SGrammarEditor::OnNodeTypeAdd)
		.OnDeleteItem(this, &SGrammarEditor::OnNodeTypeDelete)
		.OnReorderItem(this, &SGrammarEditor::OnNodeTypeReordered)
		.GetItemText(this, &SGrammarEditor::GetNodeTypeName)
		.CreateItemWidget(this, &SGrammarEditor::CreateNodeListItem)
		.IconBrush(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.NodeIcon"))
		.AllowDropOnGraph(true)
		.Title("Nodes");

    this->ChildSlot
    [
        SNew(SSplitter)
        + SSplitter::Slot()
        .Value(0.25f)
        [
            SNew(SSplitter)
            .Orientation(Orient_Vertical)
            + SSplitter::Slot()
            .Value(0.5f)
            [
                RuleListView.ToSharedRef()
            ]
            + SSplitter::Slot()
            .Value(0.5f)
            [
                NodeTypeListView.ToSharedRef()
            ]
        ]
        + SSplitter::Slot()
        .Value(0.75f)
        [
            RuleEditor.ToSharedRef()
        ]
    ];
}

void SGrammarEditor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    if (bRequestGrammarStateChanged) {
        bRequestGrammarStateChanged = false;
        if (OnGrammarStateChanged.IsBound()) {
            OnGrammarStateChanged.Execute();
        }
    }
}

////////// RuleListView Handlers //////////
void SGrammarEditor::OnRuleSelectionChanged(UGraphGrammarProduction* Item, ESelectInfo::Type SelectInfo) {
    if (PropertyEditor.IsValid()) {
        TSharedPtr<IDetailsView> PropertyEditorPtr = PropertyEditor.Pin();
        PropertyEditorPtr->SetObject(Item);
    }

    RuleEditor->SetRule(Item);
}

FText SGrammarEditor::GetRuleListRowText(UGraphGrammarProduction* InItem) const {
    FText RuleName = InItem ? InItem->RuleName : LOCTEXT("MissingRuleName", "Unknown");
    return RuleName;
}

const TArray<UGraphGrammarProduction*>* SGrammarEditor::GetRuleList() {
    return Grammar.IsValid() ? &Grammar->ProductionRules : nullptr;
}

void SGrammarEditor::OnRuleDelete(UGraphGrammarProduction* InItem) {
    FText Title = LOCTEXT("DADeleteRuleTitle", "Delete Rule?");
    EAppReturnType::Type ReturnValue = FMessageDialog::Open(EAppMsgType::YesNo, EAppReturnType::No,
                                                            LOCTEXT("DADeleteRule",
                                                                    "Are you sure you want to delete the rule?"),
                                                            &Title);

    if (ReturnValue == EAppReturnType::Yes) {
        // Delete the rule
        if (InItem && Grammar.IsValid()) {
            Grammar->ProductionRules.Remove(InItem);
            Grammar->Modify();

            NotifyGrammarStateChanged();
        }
    }
}

void SGrammarEditor::OnRuleAdd() {
    if (Grammar.IsValid()) {
        FSnapMapEditorUtils::AddNewRule(Grammar.Get());
        Grammar->Modify();

        NotifyGrammarStateChanged();
    }
}

template <typename ItemType>
void PerformReordering(ItemType Source, ItemType Dest, TArray<ItemType>& List) {
    int32 SourceIndex = -1;
    int32 DestIndex = -1;

    if (!List.Find(Dest, DestIndex)) {
        DestIndex = 0;
    }
    List.Remove(Source);
    List.Insert(Source, DestIndex);
}

void SGrammarEditor::OnRuleReordered(UGraphGrammarProduction* Source, UGraphGrammarProduction* Dest) {
    PerformReordering(Source, Dest, Grammar->ProductionRules);
    Grammar->Modify();
}

////////// End of RuleListView Handlers //////////


////////// NodeTypeListView Handlers //////////

void SGrammarEditor::OnNodeTypeSelectionChanged(UGrammarNodeType* Item, ESelectInfo::Type SelectInfo) {
    if (PropertyEditor.IsValid()) {
        TSharedPtr<IDetailsView> PropertyEditorPtr = PropertyEditor.Pin();
        PropertyEditorPtr->SetObject(Item);
    }
}

FText SGrammarEditor::GetNodeTypeName(UGrammarNodeType* InItem) const {
    if (InItem) {
        return FText::FromName(InItem->TypeName);
    }
    return LOCTEXT("InvalidNodeText", "Unknown");
}

FText SGrammarEditor::GetNodeTypeDescription(UGrammarNodeType* InItem) const {
    if (InItem) {
        return InItem->Description;
    }
    return LOCTEXT("InvalidNodeText", "Unknown");
}

const TArray<UGrammarNodeType*>* SGrammarEditor::GetNodeTypeList() {
    return Grammar.IsValid() ? &Grammar->NodeTypes : nullptr;
}

void SGrammarEditor::OnNodeTypeDelete(UGrammarNodeType* InItem) {
    FText Title = LOCTEXT("FlowEdDeleteNodeTypeTitle", "Delete Node Type?");
    EAppReturnType::Type ReturnValue = FMessageDialog::Open(EAppMsgType::YesNo, EAppReturnType::No,
                                                            LOCTEXT("FlowEdDeleteNodeType",
                                                                    "Are you sure you want to delete the node type?"),
                                                            &Title);

    if (ReturnValue == EAppReturnType::Yes) {
        // Delete the node type
        if (InItem && Grammar.IsValid()) {
            Grammar->NodeTypes.Remove(InItem);
            Grammar->Modify();

            NotifyGrammarStateChanged();

            // Force the weak uobjects refs of the removed node type object to be broken so it can be reflected in the graph editor
            CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
        }
    }
}

void SGrammarEditor::OnNodeTypeAdd() {
    if (Grammar.IsValid()) {
        FSnapMapEditorUtils::AddNodeType(Grammar.Get());
        NotifyGrammarStateChanged();
    }
}

void SGrammarEditor::OnNodeTypeReordered(UGrammarNodeType* Source, UGrammarNodeType* Dest) {
    PerformReordering(Source, Dest, Grammar->NodeTypes);
    Grammar->Modify();
}

TSharedPtr<SWidget> SGrammarEditor::CreateNodeListItem(UGrammarNodeType* InItem) {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock)
				.Text_Raw(this, &SGrammarEditor::GetNodeTypeName, InItem)
				.Font(FDungeonArchitectStyle::Get().GetFontStyle("DungeonArchitect.ListView.LargeFont"))
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .Text_Raw(this, &SGrammarEditor::GetNodeTypeDescription, InItem)
        ];
}


////////// End of RuleListView Handlers //////////


////////// Callbacks //////////
void SGrammarEditor::OnRuleGraphChanged(TWeakObjectPtr<UGraphGrammarProduction> Rule,
                                        TWeakObjectPtr<UEdGraph_Grammar> Graph) {
    NotifyGrammarStateChanged();
}

void SGrammarEditor::NotifyGrammarStateChanged() {
    bRequestGrammarStateChanged = true;
}

////////// End of Callbacks //////////


////////// Focus Actions //////////

void SGrammarEditor::FocusOnNodeType(UGrammarNodeType* NodeType) {
    FSlateApplication::Get().SetAllUserFocus(NodeTypeListView);
    NodeTypeListView->SetItemSelection(NodeType);
    NodeTypeListView->Flash();
    NodeTypeListView->Focus();
}

void SGrammarEditor::FocusOnRule(UGraphGrammarProduction* Rule) {
    RuleListView->SetItemSelection(Rule);
    RuleListView->Flash();
    RuleListView->Focus();
}

void SGrammarEditor::FocusOnGraph(UEdGraph_Grammar* Graph) {
    UGraphGrammarProduction* Rule = FDungeonGraphUtils::FindInHierarchy<UGraphGrammarProduction>(Graph);
    RuleListView->SetItemSelection(Rule);
    RuleEditor->FocusOnGraph(Graph);
}

void SGrammarEditor::FocusOnGraphNode(UEdGraphNode* Node) {
    UGraphGrammarProduction* Rule = FDungeonGraphUtils::FindInHierarchy<UGraphGrammarProduction>(Node);
    RuleListView->SetItemSelection(Rule);
    RuleEditor->FocusOnGraphNode(Node);
}

void SGrammarEditor::FlashNodeTypePanel() {
    NodeTypeListView->Flash();
}

void SGrammarEditor::FlashRulesPanel() {
    RuleListView->Flash();
}

////////// End of Focus Actions //////////


#undef LOCTEXT_NAMESPACE

