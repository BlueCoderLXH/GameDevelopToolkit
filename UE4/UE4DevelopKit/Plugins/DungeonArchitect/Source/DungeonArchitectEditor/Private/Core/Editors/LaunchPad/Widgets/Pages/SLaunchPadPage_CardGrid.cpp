//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Widgets/Pages/SLaunchPadPage_CardGrid.h"

#include "Core/Editors/LaunchPad/Actions/Impl/LaunchPadActionsImpl.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadData.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadResource.h"
#include "Core/Editors/LaunchPad/Styles/LaunchPadStyle.h"
#include "Core/Editors/LaunchPad/Widgets/SLaunchPadPage.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STileView.h"

DEFINE_LOG_CATEGORY_STATIC(LogLaunchPadPageCardGrid, Log, All);

#define LOCTEXT_NAMESPACE "LaunchPadPagesCardGrid"

void SLaunchPadPage_CardGrid::Construct(const FArguments& InArgs, const FLaunchPadPageLayout_CardGrid& InData) {
    Data = InData;

    TSharedPtr<SVerticalBox> Host = SNew(SVerticalBox);

    // Add the title
    if (Data.Title.Len() > 0) {
        static const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 18);
        Host->AddSlot()
            .Padding(FMargin(0, 0, 0, 8))
            .AutoHeight()
        [
            SNew(STextBlock)
			.Text(FText::FromString(Data.Title))
			.Font(TitleFont)
        ];
    }

    // Add the description
    if (Data.Description.Len() > 0) {
        Host->AddSlot()
            .Padding(FMargin(0, 0, 0, 8))
            .AutoHeight()
        [
            SNew(STextBlock)
			.Text(FText::FromString(Data.Description))
			.AutoWrapText(true)
        ];
    }

    const float CardWidth = Data.ImageWidth + Data.CardPadding * 2;
    FLaunchPadPage_CardGridItemSettings CardSettings;
    CardSettings.CardWidth = CardWidth;
    CardSettings.CardHeight = Data.CardHeight;
    CardSettings.CardPadding = Data.CardPadding;
    CardSettings.ImageWidth = Data.ImageWidth;
    CardSettings.ImageHeight = Data.ImageHeight;
    CardSettings.bShowCategoryTitles = Data.ShowCategoryTitles;

    // Add the category card items
    for (const FLaunchPadPageLayout_CardGrid_Category& Category : Data.Categories) {
        Host->AddSlot()
            .AutoHeight()
        [
            SNew(SLaunchPadPage_CardGridCategory, Category)
			.CardSettings(CardSettings)
			.Parent(SharedThis(this))
        ];
    }

    ChildSlot
    [
        SNew(SBox)
        .Padding(FMargin(10, 0, 0, 0))
        [
            Host.ToSharedRef()
        ]
    ];
}

FString SLaunchPadPage_CardGrid::GetTitle() {
    return Data.Title;
}

void SLaunchPadPage_CardGridCategory::OnMouseButtonClicked(TSharedPtr<FLaunchPadPageLayout_CardGrid_CardInfo> InCard) {
    //UE_LOG(LogLaunchPadPageCardGrid, Log, TEXT("Item Clicked: %s"), InCard.IsValid() ? *InCard->Link : TEXT("[NONE]"));
    if (InCard.IsValid()) {
        if (InCard->Url != "") {
            FLaunchPadActions::Exec_OpenURL(InCard->Url);
        }
        else if (Parent.IsValid()) {
            TSharedPtr<SLaunchPadPage_CardGrid> ParentPtr = Parent.Pin();
            ParentPtr->GetOnLinkClicked().ExecuteIfBound(InCard->Link);
        }
    }
}

void SLaunchPadPage_CardGridCategory::Construct(const FArguments& InArgs,
                                                const FLaunchPadPageLayout_CardGrid_Category& InCategoryData) {
    CategoryData = InCategoryData;
    CardSettings = InArgs._CardSettings;
    Parent = InArgs._Parent;

    static const FSlateFontInfo CategoryFont = FCoreStyle::GetDefaultFontStyle("Bold", 16);

    CardItems.Reset();
    for (const FLaunchPadPageLayout_CardGrid_CardInfo& Card : CategoryData.Cards) {
        CardItems.Add(MakeShareable(new FLaunchPadPageLayout_CardGrid_CardInfo(Card)));
    }

    TSharedPtr<SVerticalBox> Host = SNew(SVerticalBox);

    if (CardSettings.bShowCategoryTitles) {
        Host->AddSlot()
            .AutoHeight()
        [
            SNew(SBox)
            .Padding(FMargin(0, 16, 0, 0))
            [
                SNew(STextBlock)
				.Text(FText::FromString(CategoryData.Category))
				.Font(CategoryFont)
            ]
        ];

        Host->AddSlot()
            .AutoHeight()
        [
            SNew(SSeparator)
        ];
    }
    else {
        Host->AddSlot()
            .AutoHeight()
        [
            SNew(SSpacer)
            .Size(FVector2D(16, 16))
        ];
    }

    Host->AddSlot()
        .AutoHeight()
    [
        SNew(STileView<TSharedPtr<FLaunchPadPageLayout_CardGrid_CardInfo>>)
		.ListItemsSource(&CardItems)
		.OnGenerateTile(this, &SLaunchPadPage_CardGridCategory::GenerateCardWidget)
		.OnMouseButtonClick(this, &SLaunchPadPage_CardGridCategory::OnMouseButtonClicked)
		.ItemAlignment(EListItemAlignment::LeftAligned)
		.ItemWidth(CardSettings.CardWidth)
		.ItemHeight(CardSettings.CardHeight)
    ];

    ChildSlot
    [
        Host.ToSharedRef()
    ];
}

TSharedRef<class ITableRow> SLaunchPadPage_CardGridCategory::GenerateCardWidget(
    TSharedPtr<FLaunchPadPageLayout_CardGrid_CardInfo> InItem, const TSharedRef<class STableViewBase>& OwnerTable) {
    return SNew(STableRow<TSharedPtr<FLaunchPadCategoryItem>>, OwnerTable)
    [
        SNew(SLaunchPadPage_CardGridItemWidget, CardSettings, InItem)
    ];
}

void SLaunchPadPage_CardGridItemWidget::Construct(const FArguments& Args,
                                                  const FLaunchPadPage_CardGridItemSettings& InCardSettings,
                                                  TSharedPtr<FLaunchPadPageLayout_CardGrid_CardInfo> InItem) {
    static const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 12);
    FString Title = InItem.IsValid() ? InItem->Title : "";
    FString Description = InItem.IsValid() ? InItem->Desc : "";
    if (InItem.IsValid()) {
        bIsHyperlink = (InItem->Url.Len() > 0);
    }

    const FSlateBrush* ThumbBrush = FDALaunchPadStyle::Get().GetBrush(*InItem->Image);

    ChildSlot
    [
        SNew(SBox)
        .Padding(FMargin(InCardSettings.CardPadding))
        [
            SNew(SBorder)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SBox)
					.WidthOverride(InCardSettings.ImageWidth)
					.HeightOverride(InCardSettings.ImageHeight)
                    [
                        SNew(SImage)
                        .Image(ThumbBrush)
                    ]
                ]
                + SVerticalBox::Slot()
                  .Padding(0, 4, 0, 0)
                  .AutoHeight()
                [
                    SNew(STextBlock)
					.Font(TitleFont)
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
					.Text(FText::FromString(Title))
                ]
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SNew(STextBlock)
					.Text(FText::FromString(Description))
					.AutoWrapText(true)
					.Justification(ETextJustify::Center)
                ]
            ]
        ]
    ];

}

FCursorReply SLaunchPadPage_CardGridItemWidget::OnCursorQuery(const FGeometry& MyGeometry,
                                                              const FPointerEvent& CursorEvent) const {
    if (bIsHyperlink) {
        return FCursorReply::Cursor(EMouseCursor::Hand);
    }
    return SCompoundWidget::OnCursorQuery(MyGeometry, CursorEvent);
}

#undef LOCTEXT_NAMESPACE

