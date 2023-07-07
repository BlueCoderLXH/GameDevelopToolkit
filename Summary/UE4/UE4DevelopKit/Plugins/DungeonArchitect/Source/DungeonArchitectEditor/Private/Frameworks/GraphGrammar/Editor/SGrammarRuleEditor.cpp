//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Editor/SGrammarRuleEditor.h"

#include "Core/Editors/SnapMapEditor/SnapMapEditorUtils.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Frameworks/GraphGrammar/Editor/SGrammarRuleGraph.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"
#include "Frameworks/GraphGrammar/Script/GrammarRuleScript.h"

#include "EditorStyleSet.h"
#include "Layout/Children.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "GrammarRuleEditor"

void SGrammarRuleEditor::Construct(const FArguments& InArgs) {
    Rule = InArgs._Rule;
    PropertyEditor = InArgs._PropertyEditor;
    OnRuleGraphChanged = InArgs._OnRuleGraphChanged;

    RHSWidgets = SNew(SVerticalBox);
    LHSWidget = SNew(SGrammarRuleGraph, nullptr)
				.Title("LHS")
				.bShowCloseButton(false)
				.bShowWeight(false)
				.PropertyEditor(PropertyEditor)
				.DropHandler(MakeShareable(new FNodeTypeGraphDropHandler))
				.OnGraphChanged_Raw(this, &SGrammarRuleEditor::OnGraphChangedCallback);


    this->ChildSlot
    [
        SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(6.0f))
        [
            SAssignNew(ScrollBox, SScrollBox)
            + SScrollBox::Slot()
            [
                SNew(SBox)
                .Padding(6)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    [
                        SNew(STextBlock)
						.Text_Raw(this, &SGrammarRuleEditor::GetRuleName)
						.Font(FDungeonArchitectStyle::Get().GetFontStyle("DungeonArchitect.ListView.LargeFont"))
                    ]
                ]
            ]

            // LHS
            + SScrollBox::Slot()
            [
                LHSWidget.ToSharedRef()
            ]

            // Production arrow image
            + SScrollBox::Slot()
            [
                SNew(SBorder)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.BorderBackgroundColor(FColor::Transparent)
                [
                    SNew(SImage)
                    .Image(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.RuleArrow"))
                ]
            ]

            // RHS List
            + SScrollBox::Slot()
            [
                RHSWidgets.ToSharedRef()
            ]

            // Add new RHS button
            + SScrollBox::Slot()
            [
                SNew(SBox)
                .HeightOverride(32)
                [
                    SNew(SButton)
					.ButtonStyle(
                                     &FDungeonArchitectStyle::Get().GetWidgetStyle<FButtonStyle>(
                                         "DungeonArchitect.FlatButton.Blue"))
					.OnClicked(this, &SGrammarRuleEditor::OnAddRHSRuleClicked)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
                    [
                        SNew(SImage)
                        .Image(FEditorStyle::Get().GetBrush("Plus"))
                    ]
                ]
            ]
        ]
    ];
}

void SGrammarRuleEditor::SetRule(TWeakObjectPtr<UGraphGrammarProduction> InRule) {
    if (Rule == InRule) {
        return;
    }
    Rule = InRule;
    if (Rule.IsValid()) {
        // Set the LHS Graph
        LHSWidget->SetGraph(FSnapMapEditorUtils::GetEdGraph(Rule->SourceGraph));

        // Set the RHS Graphs
        RHSWidgets->ClearChildren();
        for (UGrammarRuleScript* RHSGraph : InRule->DestGraphs) {
            AddRHSWidget(FSnapMapEditorUtils::GetEdGraph(RHSGraph));
        }
    }
}

TSharedPtr<SGrammarRuleGraph> SGrammarRuleEditor::FindGraphWidget(UEdGraph_Grammar* Graph) {
    if (LHSWidget->GetGraph() == Graph) return LHSWidget;
    FChildren* Children = RHSWidgets->GetChildren();
    for (int i = 0; i < RHSWidgets->NumSlots(); i++) {
        TSharedRef<SGrammarRuleGraph> RHSWidget = StaticCastSharedRef<SGrammarRuleGraph>(Children->GetChildAt(i));
        if (RHSWidget->GetGraph() == Graph) return RHSWidget;
    }
    return nullptr;
}

void SGrammarRuleEditor::OnGraphChangedCallback(TWeakObjectPtr<UEdGraph_Grammar> Graph) {
    NotifyRuleChanged(Graph);
}

void SGrammarRuleEditor::NotifyRuleChanged(TWeakObjectPtr<UEdGraph_Grammar> Graph) {
    if (OnRuleGraphChanged.IsBound()) {
        OnRuleGraphChanged.Execute(Rule, Graph);
    }
}

void SGrammarRuleEditor::FocusOnGraph(UEdGraph_Grammar* Graph) {
    TSharedPtr<SGrammarRuleGraph> GraphWidget = FindGraphWidget(Graph);
    if (!GraphWidget.IsValid()) {
        return;
    }

    ScrollBox->ScrollDescendantIntoView(GraphWidget);
    GraphWidget->Flash();
    GraphWidget->Focus();
}

void SGrammarRuleEditor::FocusOnGraphNode(UEdGraphNode* GrammarNode) {
    UEdGraph_Grammar* Graph = Cast<UEdGraph_Grammar>(GrammarNode->GetGraph());
    TSharedPtr<SGrammarRuleGraph> GraphWidget = FindGraphWidget(Graph);
    if (!GraphWidget.IsValid()) {
        return;
    }

    ScrollBox->ScrollDescendantIntoView(GraphWidget);
    GraphWidget->Flash();
    GraphWidget->Focus();

    GraphWidget->JumpToNode(GrammarNode);
}

void SGrammarRuleEditor::AddRHSWidget(UEdGraph_Grammar* RHSGraph) {
    SVerticalBox::FSlot& Slot = RHSWidgets->AddSlot();
    Slot.AutoHeight();
    Slot.AttachWidget(
        SNew(SGrammarRuleGraph, RHSGraph)
		.Title("RHS")
		.PropertyEditor(PropertyEditor)
		.DropHandler(MakeShareable(new FNodeTypeGraphDropHandler))
		.OnDelete_Raw(this, &SGrammarRuleEditor::OnDeleteRHSRuleClicked)
		.OnGraphChanged_Raw(this, &SGrammarRuleEditor::OnGraphChangedCallback)
    );

    NotifyRuleChanged(RHSGraph);
}

void SGrammarRuleEditor::DeleteRHSGraph(TWeakObjectPtr<UEdGraph_Grammar> RHSGraph) {
    // Find the slot this graph belongs to graph
    for (int i = 0; i < RHSWidgets->NumSlots(); i++) {
        TSharedRef<SGrammarRuleGraph> GraphWidget = StaticCastSharedRef<SGrammarRuleGraph>(
            RHSWidgets->GetChildren()->GetChildAt(i));
        if (GraphWidget->GetGraph() == RHSGraph) {
            // Remove the widget
            RHSWidgets->RemoveSlot(GraphWidget);
            break;
        }
    }

    // Remove the graph from the rule
    UGrammarRuleScript* ScriptOwner = RHSGraph.IsValid() ? Cast<UGrammarRuleScript>(RHSGraph->GetOuter()) : nullptr;
    if (ScriptOwner) {
        Rule->DestGraphs.Remove(ScriptOwner);
    }

    NotifyRuleChanged(RHSGraph);
}

FReply SGrammarRuleEditor::OnDeleteRHSRuleClicked(TWeakObjectPtr<UEdGraph_Grammar> RHSGraph) {
    FText Title = LOCTEXT("DADeleteRHSRuleTitle", "Delete Rule Graph?");
    EAppReturnType::Type ReturnValue = FMessageDialog::Open(EAppMsgType::YesNo, EAppReturnType::No,
                                                            LOCTEXT("DADeleteRHSRule",
                                                                    "Are you sure you want to delete the rule graph?"),
                                                            &Title);

    if (ReturnValue == EAppReturnType::Yes) {
        DeleteRHSGraph(RHSGraph);
    }

    return FReply::Handled();
}

FText SGrammarRuleEditor::GetRuleName() const {
    FText Name = Rule.IsValid() ? Rule->RuleName : LOCTEXT("MissingRuleName", "Unknown");
    FFormatNamedArguments Args;
    Args.Add(TEXT("RuleName"), Name);
    return FText::Format(LOCTEXT("GrammarRuleEditorTitle", "Rule Graph [{RuleName}]"), Args);
}

FReply SGrammarRuleEditor::OnAddRHSRuleClicked() {
    if (Rule.IsValid()) {
        UGrammarRuleScript* NewRHSGraphScript = FSnapMapEditorUtils::AddProductionRHSGraph(Rule.Get());
        AddRHSWidget(FSnapMapEditorUtils::GetEdGraph(NewRHSGraphScript));

        Rule->GetOutermost()->Modify();
    }
    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

