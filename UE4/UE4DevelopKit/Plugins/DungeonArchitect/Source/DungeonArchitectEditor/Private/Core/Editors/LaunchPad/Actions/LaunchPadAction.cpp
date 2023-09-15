//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Actions/LaunchPadAction.h"

#include "Core/Editors/LaunchPad/Actions/Impl/LaunchPadActionsImpl.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadData.h"
#include "Core/Editors/LaunchPad/Styles/LaunchPadStyle.h"

#include "EditorStyleSet.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SLaunchPadAction"

void SLaunchPadAction::Construct(const FArguments& InArgs, const FLaunchPadPageActionData& InAction) {
    Action = InAction;
    FName BrushKey = GetBrushName();
    FText Title = GetTitle();

    const FButtonStyle* ButtonStyle =
        (Action.Type == ELaunchPadActionType::LauncherURL || Action.Type == ELaunchPadActionType::AddStarterContent)
            ? &FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Success")
            : &FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Primary");

    ChildSlot
    [
        SNew(SBox)
        .WidthOverride(this, &SLaunchPadAction::GetButtonWidth)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SButton)
				.ButtonStyle(ButtonStyle)
				.OnClicked(this, &SLaunchPadAction::OnButtonClicked)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()[SNullWidget::NullWidget]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SBox)
							.WidthOverride(36)
							.HeightOverride(36)
                            [
                                SNew(SImage)
                                .Image(FDALaunchPadStyle::Get().GetBrush(BrushKey))
                            ]
                        ]
                        + SHorizontalBox::Slot()[SNullWidget::NullWidget]
                    ]
                    + SVerticalBox::Slot()
                      .Padding(FMargin(0, 6, 0, 0))
                      .AutoHeight()
                    [
                        SNew(STextBlock)
						.Text(Title)
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
                    ]
                ]
            ]
            + SVerticalBox::Slot()[SNullWidget::NullWidget]
        ]
    ];
}

FReply SLaunchPadAction::OnButtonClicked() {
    FLaunchPadActions::Execute(Action, AsShared());
    return FReply::Handled();
}

FText SLaunchPadAction::GetTitle() const {
    if (!Action.Title.IsEmpty()) {
        return FText::FromString(Action.Title);
    }

    switch (Action.Type) {
    case ELaunchPadActionType::OpenFolder:
        return LOCTEXT("ButtonLabel_OpenFolder", "Open Folder");

    case ELaunchPadActionType::OpenScene:
        return LOCTEXT("ButtonLabel_OpenScene", "Open Scene");

    case ELaunchPadActionType::OpenTheme:
        return LOCTEXT("ButtonLabel_OpenTheme", "Open Theme");

    case ELaunchPadActionType::OpenSnapFlow:
        return LOCTEXT("ButtonLabel_OpenSnapFlow", "Open SnapFlow");

    case ELaunchPadActionType::OpenGridFlow:
        return LOCTEXT("ButtonLabel_OpenGridFlow", "Open GridFlow");

    case ELaunchPadActionType::CloneScene:
    case ELaunchPadActionType::CloneSceneAndBuild:
        return LOCTEXT("ButtonLabel_CloneScene", "Clone Scene");

    case ELaunchPadActionType::CloneTheme:
        return LOCTEXT("ButtonLabel_CloneTheme", "Clone Theme");

    case ELaunchPadActionType::CloneSnapFlow:
        return LOCTEXT("ButtonLabel_CloneSnapFlow", "Clone SnapFlow");

    case ELaunchPadActionType::CloneGridFlow:
        return LOCTEXT("ButtonLabel_CloneGridFlow", "Clone GridFlow");

    case ELaunchPadActionType::CloneSnapGridFlow:
        return LOCTEXT("ButtonLabel_CloneSnapGridFlow", "Clone Snap Grid Flow");
        
    case ELaunchPadActionType::Documentation:
        return LOCTEXT("ButtonLabel_Documentation", "Docs");

    case ELaunchPadActionType::Video:
        return LOCTEXT("ButtonLabel_Video", "Video");

    case ELaunchPadActionType::LauncherURL:
        return LOCTEXT("ButtonLabel_LauncherURL", "Download");

    case ELaunchPadActionType::AddStarterContent:
        return LOCTEXT("ButtonLabel_LauncherURL", "Add Starter Content");

    case ELaunchPadActionType::None:
    default:
        return LOCTEXT("ButtonLabel_None", "Invalid Input");
    }
}

FName SLaunchPadAction::GetBrushName() const {
    if (!Action.Icon.IsEmpty()) {
        return *Action.Icon;
    }

    switch (Action.Type) {
    case ELaunchPadActionType::OpenFolder:
        return TEXT("icons/icon_open_folder");

    case ELaunchPadActionType::OpenScene:
        return TEXT("icons/icon_open_scene");

    case ELaunchPadActionType::OpenTheme:
        return TEXT("icons/icon_theme");

    case ELaunchPadActionType::OpenSnapFlow:
        return TEXT("icons/icon_graph");

    case ELaunchPadActionType::OpenGridFlow:
        return TEXT("icons/icon_gridflow");

    case ELaunchPadActionType::CloneScene:
    case ELaunchPadActionType::CloneSceneAndBuild:
        return TEXT("icons/icon_clone_scene");

    case ELaunchPadActionType::CloneTheme:
        return TEXT("icons/icon_theme_new");

    case ELaunchPadActionType::CloneSnapFlow:
        return TEXT("icons/icon_graph_new");

    case ELaunchPadActionType::CloneGridFlow:
        return TEXT("icons/icon_gridflow_new");

    case ELaunchPadActionType::CloneSnapGridFlow:
        return TEXT("icons/icon_gridflow_new");
        
    case ELaunchPadActionType::Documentation:
        return TEXT("icons/icon_docs");

    case ELaunchPadActionType::Video:
        return TEXT("icons/icon_video");

    case ELaunchPadActionType::LauncherURL:
        return TEXT("icons/icon_download");

    case ELaunchPadActionType::AddStarterContent:
        return TEXT("icons/icon_download");

    case ELaunchPadActionType::None:
    default:
        return TEXT("icons/icon_open_folder");
    }

}

FOptionalSize SLaunchPadAction::GetButtonWidth() const {
    return Action.Width > 0 ? Action.Width : 68;
}

#undef LOCTEXT_NAMESPACE

