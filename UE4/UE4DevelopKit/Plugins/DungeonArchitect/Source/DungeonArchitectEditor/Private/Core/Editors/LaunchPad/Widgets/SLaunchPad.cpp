//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Widgets/SLaunchPad.h"

#include "Core/Editors/LaunchPad/Data/LaunchPadData.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadResource.h"
#include "Core/Editors/LaunchPad/Widgets/SLaunchPadBreadCrumb.h"
#include "Core/Editors/LaunchPad/Widgets/SLaunchPadCategories.h"
#include "Core/Editors/LaunchPad/Widgets/SLaunchPadPage.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#include "EditorStyleSet.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"

#define LOCTEXT_NAMESPACE "HelpSystemList"

DEFINE_LOG_CATEGORY_STATIC(LogLaunchPad, Log, All);

void SLaunchPad::Construct(const FArguments& Args) {
    DataSource = MakeShareable(new FLaunchPadResourceFolderDataSource);
    static const FSlateFontInfo BannerFont = FCoreStyle::GetDefaultFontStyle("Bold", 16);

    ChildSlot
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SOverlay)
            +SOverlay::Slot()
            [
                SNew(SBorder)
                .BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
                .BorderBackgroundColor(FLinearColor(0.5f, 0.5f, 0.5f))
                [
                    SAssignNew(Categories, SLaunchPadCategories)
                    .OnSelectionChanged(this, &SLaunchPad::OnCategorySelectionChanged)
                ]
            ]
            +SOverlay::Slot()
            [
                CreateSidebarLogo().ToSharedRef()
            ]
        ]
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .Padding(5)
            .AutoHeight()
            [
                SNew(SBorder)
                .BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
                .BorderBackgroundColor(FLinearColor(0.75f, 0.75f, 0.75f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("LaunchPadBannerText", "Dungeon Architect - Launch Pad"))
                    .Font(BannerFont)
                    .Justification(ETextJustify::Center)
                    .ShadowOffset(FVector2D(1, 2))
                    .ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.75f))
                ]
            ]
            
            + SVerticalBox::Slot()
            .Padding(6, 0, 0, 6)
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SAssignNew(BreadCrumb, SLaunchPadBreadCrumb)
                    .OnNavigation(this, &SLaunchPad::OnBreadCrumbNavigation)
                ]
                +SHorizontalBox::Slot()
                [
                    SNullWidget::NullWidget
                ]
            ]

            +SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SNew(SScrollBox)
                +SScrollBox::Slot()
                [
                    SNew(SBorder)
                    .BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
                    [
                        SAssignNew(PageHost, SBox)
                    ]
                ]
            ]
        ]
    ];

    Categories->Refresh(DataSource);
}

TSharedPtr<SLaunchPadPage> SLaunchPad::CreatePage(const FString& InPath) {
    TSharedPtr<SLaunchPadPage> Page = FLaunchPadPageWidgetFactory::Create(DataSource, InPath);
    if (Page.IsValid()) {
        Page->GetOnLinkClicked().BindRaw(this, &SLaunchPad::OnLinkClicked);
    }
    return Page;
}

void SLaunchPad::OnLinkClicked(const FString& InPath) {
    TSharedPtr<SLaunchPadPage> Page = CreatePage(InPath);
    if (Page.IsValid()) {
        BreadCrumb->PushPage(Page);
    }
}

TSharedPtr<SWidget> SLaunchPad::CreateSidebarLogo() const {
    return
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [
            SNullWidget::NullWidget
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            [
                SNullWidget::NullWidget
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SBox)
				.WidthOverride(128)
				.HeightOverride(128)
				.Visibility(EVisibility::HitTestInvisible)
                [
                    SNew(SImage)
					.Image(FDungeonArchitectStyle::Get().GetBrush("DungeonArchitect.LaunchPad.IconSideBar"))
					.ColorAndOpacity(FLinearColor(1, 1, 1, 0.1f))
                ]
            ]
            + SHorizontalBox::Slot()
            [
                SNullWidget::NullWidget
            ]
        ];
}

void SLaunchPad::OnCategorySelectionChanged(TSharedPtr<FLaunchPadCategoryItem> SelectedItem,
                                            ESelectInfo::Type SelectInfo) {
    UE_LOG(LogLaunchPad, Log, TEXT("Category: %s"), SelectedItem.IsValid() ? *SelectedItem->Title : TEXT("[NONE]"));
    if (SelectedItem.IsValid()) {
        TSharedPtr<SLaunchPadPage> Page = CreatePage(SelectedItem->Page);
        if (Page.IsValid()) {
            BreadCrumb->SetRoot(Page);
        }
    }
}

void SLaunchPad::OnBreadCrumbNavigation(TSharedPtr<SLaunchPadPage> InPage) {
    if (InPage.IsValid()) {
        PageHost->SetContent(InPage.ToSharedRef());
    }
}

#undef LOCTEXT_NAMESPACE

