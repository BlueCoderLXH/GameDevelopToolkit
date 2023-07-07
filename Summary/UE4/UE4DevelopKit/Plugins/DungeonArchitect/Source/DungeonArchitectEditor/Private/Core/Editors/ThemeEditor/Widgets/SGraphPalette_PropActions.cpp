//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/SGraphPalette_PropActions.h"

#include "Core/Editors/ThemeEditor/DungeonArchitectThemeEditor.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphSchema_DungeonProp.h"

void SGraphPalette_PropActions::Construct(const FArguments& InArgs,
                                          TWeakPtr<FDungeonArchitectThemeEditor> InDungeonEditor) {
    this->DungeonEditor = InDungeonEditor;

    this->ChildSlot
    [
        SNew(SBorder)
			.Padding(2.0f)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
        [
            SNew(SVerticalBox)

            // Content list
            + SVerticalBox::Slot()
            [
                SNew(SOverlay)

                + SOverlay::Slot()
                  .HAlign(HAlign_Fill)
                  .VAlign(VAlign_Fill)
                [
                    //SNullWidget::NullWidget
                    SAssignNew(GraphActionMenu, SGraphActionMenu)
						.OnActionDragged(this, &SGraphPalette_PropActions::OnActionDragged)
						.OnCreateWidgetForAction(this, &SGraphPalette_PropActions::OnCreateWidgetForAction)
						.OnCollectAllActions(this, &SGraphPalette_PropActions::CollectAllActions)
						.AutoExpandActionMenu(true)
                ]
            ]
        ]
    ];
}

void SGraphPalette_PropActions::CollectAllActions(FGraphActionListBuilderBase& OutAllActions) {
    const UEdGraphSchema_DungeonProp* Schema = GetDefault<UEdGraphSchema_DungeonProp>();
    TArray<TSharedPtr<FEdGraphSchemaAction>> Actions;
    FGraphActionMenuBuilder ActionMenuBuilder;
    if (DungeonEditor.IsValid()) {
        TSharedPtr<SGraphEditor> GraphEditor = DungeonEditor.Pin()->GetGraphEditor();
        if (GraphEditor.IsValid()) {
            UEdGraph* Graph = GraphEditor->GetCurrentGraph();
            Schema->GetActionList(Actions, Graph, Graph);
            for (TSharedPtr<FEdGraphSchemaAction> Action : Actions) {
                ActionMenuBuilder.AddAction(Action);
            }
        }
    }
    OutAllActions.Append(ActionMenuBuilder);
}

void SGraphPalette_PropActions::Refresh() {
    RefreshActionsList(true);
}

TSharedRef<SWidget> SGraphPalette_PropActions::OnCreateWidgetForAction(FCreateWidgetForActionData* const InCreateData) {
    return SGraphPalette::OnCreateWidgetForAction(InCreateData);
}

FReply SGraphPalette_PropActions::OnActionDragged(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions,
                                                  const FPointerEvent& MouseEvent) {
    return SGraphPalette::OnActionDragged(InActions, MouseEvent);
}

