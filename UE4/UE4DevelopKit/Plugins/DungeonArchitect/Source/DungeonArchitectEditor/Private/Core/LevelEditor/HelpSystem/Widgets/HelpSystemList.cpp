//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/HelpSystem/Widgets/HelpSystemList.h"

#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystemStyle.h"

#include "EditorStyleSet.h"
#include "Framework/SlateDelegates.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "HelpSystemList"

void SHelpSystemList::Construct(const FArguments& InArgs) {
    FText Title = FText::FromString("Test Title");
    FText Description = FText::FromString("Test Description");

    float ItemHeight = 80;

    struct Local {
        static FReply OpenURL(FString Link) {
            FPlatformProcess::LaunchURL(*Link, nullptr, nullptr);
            return FReply::Handled();
        }
    };

    static const FString UrlUserGuide = "http://coderespawn.github.io/dungeon-architect-user-guide-ue4";
    static const FString UrlQuickStartGuide = "http://coderespawn.github.io/dungeon-architect-quick-start-ue4";
    static const FString UrlVideoTutorials =
        "http://htmlpreview.github.io/?https://github.com/coderespawn/dungeon-architect-quick-start-ue4/blob/4.9/Docs/VideoTutorials.html";
    static const FString UrlReleaseNotes = "https://gist.github.com/coderespawn/590e8bbb4adc9782cab8e48ae2a64864";
    static const FString UrlDiscord = "https://discord.gg/dRewTSU";
    static const FString UrlForumThread = "https://forums.unrealengine.com/showthread.php?67599-Dungeon-Architect";
    static const FString UrlRatePlugin = "https://www.unrealengine.com/marketplace/dungeon-architect";

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("UserGuideTitle", "User Guide"),
                LOCTEXT("UserGuideDesc", "Learn how to use Dungeon Architect with this user guide"),
                "DungeonArchitect.HelpSystem.IconUserGuide",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlUserGuide)
            ).ToSharedRef()
        ]
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("QuickStartTitle", "Quick Start Guide"),
                LOCTEXT("QuickStartDesc",
                        "Use the rich set of samples and demos in this guide to quickly get started with Dungeon Architect"),
                "DungeonArchitect.HelpSystem.IconQuickStartGuide",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlQuickStartGuide)
            ).ToSharedRef()
        ]
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("VideoTutorialsTitle", "Video Tutorials"),
                LOCTEXT("VideoTutorialsDesc", "Watch the video tutorials"),
                "DungeonArchitect.HelpSystem.IconVideoTutorials",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlVideoTutorials)
            ).ToSharedRef()
        ]
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("ChangeLogTitle", "Release Notes"),
                LOCTEXT("ChangeLogDesc", "Find what\'s new in the latest release of the plugin"),
                "DungeonArchitect.HelpSystem.IconReleaseNotes",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlReleaseNotes)
            ).ToSharedRef()
        ]
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("DiscordCommunityTitle", "Discord Community"),
                LOCTEXT("DiscordCommunityDesc",
                        "Join our Discord channel and chat with the developer and the community"),
                "DungeonArchitect.HelpSystem.IconDiscordChat",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlDiscord)
            ).ToSharedRef()
        ]
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("ForumThreadTitle", "Forums Support Thread / Dev Log"),
                LOCTEXT("ForumThreadDesc",
                        "Follow the latest progress of Dungeon Architect and discuss anything regarding the plugin in the forum thread"),
                "DungeonArchitect.HelpSystem.IconForumThread",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlForumThread)
            ).ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("RateTitle", "Rate us in the Marketplace"),
                LOCTEXT("RateDesc", "Let us know what you think of the plugin by rating in the marketplace"),
                "DungeonArchitect.HelpSystem.IconRatePlugin",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlRatePlugin)
            ).ToSharedRef()
        ]
    ];
}


TSharedPtr<SWidget> SHelpSystemList::CreateEntry(const FText& Title, const FText& Description, const FName& IconId,
                                                 FOnClicked OnClicked) {
    //const FSlateBrush* SlateBrush = FEditorStyle::Get().GetBrush("Tutorials.Browser.DefaultTutorialIcon");
    const FSlateBrush* SlateBrush = FDungeonArchitectHelpSystemStyle::Get().GetBrush(IconId);
    return
        SNew(SBox)
        .Padding(FMargin(0.0f, 2.0f))
        [
            SNew(SButton)
			.OnClicked(OnClicked)
			.ButtonStyle(&FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("Tutorials.Browser.Button"))
			.ForegroundColor(FSlateColor::UseForeground())
			.Content()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                  .AutoWidth()
                  .VAlign(VAlign_Center)
                  .HAlign(HAlign_Center)
                  .Padding(8.0f)
                [
                    SNew(SOverlay)
                    + SOverlay::Slot()
                    [
                        SNew(SBox)
						.WidthOverride(64.0f)
						.HeightOverride(64.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
                        [
                            SNew(SImage)
                            .Image(SlateBrush)
                        ]
                    ]
                ]
                + SHorizontalBox::Slot()
                  .FillWidth(1.0f)
                  .VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
						.Text(Title)
						.TextStyle(&FEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>(
                                            "Tutorials.Browser.SummaryHeader"))
						//.HighlightText(HighlightText)
						.HighlightColor(FEditorStyle::Get().GetColor("Tutorials.Browser.HighlightTextColor"))
						.HighlightShape(FEditorStyle::Get().GetBrush("TextBlock.HighlightShape"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    [
                        SNew(STextBlock)
						.AutoWrapText(true)
						.Text(Description)
						.TextStyle(
                                            &FEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>(
                                                "Tutorials.Browser.SummaryText"))
						//.HighlightText(HighlightText)
						.HighlightColor(FEditorStyle::Get().GetColor("Tutorials.Browser.HighlightTextColor"))
						.HighlightShape(FEditorStyle::Get().GetBrush("TextBlock.HighlightShape"))
                    ]
                ]
            ]
        ];
}

#undef LOCTEXT_NAMESPACE

