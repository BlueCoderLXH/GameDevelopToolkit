//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/SMarkerListView.h"

#include "EditorStyleSet.h"
#include "Types/ISlateMetaData.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SMarkerListView::Construct(const FArguments& InArgs) {
    OnMarkerDoubleClicked = InArgs._OnMarkerDoubleClicked;

    MarkersListView = SNew(FMarkerListView)
		.ItemHeight(24)
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&FilteredMarkerListEntries)
		.OnGenerateRow(this, &SMarkerListView::OnGenerateMarkerListRow)
		.OnMouseButtonDoubleClick(this, &SMarkerListView::OnMarkerListDoubleClicked);

    const FSlateBrush* InfoIcon = FEditorStyle::GetBrush("Icons.Info");
    FText InfoText = FText::FromString("Double click to jump to the node");

    this->ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("SettingsEditor.CheckoutWarningBorder"))
			.BorderBackgroundColor(FColor(32, 32, 32))
            [
                SNew(SHorizontalBox)
                .Visibility(EVisibility::Visible)

                + SHorizontalBox::Slot()
                  .VAlign(VAlign_Center)
                  .AutoWidth()
                  .Padding(4.0f, 0.0f, 0.0f, 0.0f)
                [
                    SNew(SImage)
                    .Image(InfoIcon)
                ]

                + SHorizontalBox::Slot()
                  .VAlign(VAlign_Center)
                  .AutoWidth()
                  .Padding(4.0f, 0.0f, 0.0f, 0.0f)
                [
                    SNew(STextBlock)
                    .Text(InfoText)
                ]
            ]
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SSearchBox)
			.OnTextChanged(this, &SMarkerListView::OnFilterTextChanged)
			.OnTextCommitted(this, &SMarkerListView::OnFilterTextCommitted)
        ]
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SBorder)
			.Padding(4.0f)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            [
                SNew(SBox)
                .AddMetaData<FTagMetaData>(FTagMetaData(TEXT("Markers")))
                [
                    MarkersListView.ToSharedRef()
                ]
            ]
        ]
    ];

}

TSharedRef<ITableRow> SMarkerListView::OnGenerateMarkerListRow(TSharedPtr<FMarkerListEntry> InItem,
                                                               const TSharedRef<STableViewBase>& OwnerTable) {
    return SNew(STableRow<TSharedPtr<ITableRow> >, OwnerTable)
        .Content()
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
              .AutoWidth()
              .Padding(2, 2)
            [
                SNew(STextBlock)
                .Text(FText::FromString(InItem->MarkerName))
            ]
        ];
}

void SMarkerListView::OnMarkerListDoubleClicked(TSharedPtr<FMarkerListEntry> Entry) {
    if (OnMarkerDoubleClicked.IsBound()) {
        OnMarkerDoubleClicked.Execute(Entry);
    }
}

void SMarkerListView::OnFilterTextChanged(const FText& InFilterText) {
    FilterText = InFilterText;
    RefreshImpl();
}

void SMarkerListView::OnFilterTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType) {
    FilterText = InFilterText;
    RefreshImpl();
}

void SMarkerListView::Refresh(TArray<TSharedPtr<FMarkerListEntry>> InMarkerListEntries) {
    MarkerListEntries = InMarkerListEntries;
    RefreshImpl();
}

void SMarkerListView::RefreshImpl() {
    // Apply the filter on the text
    FString FilterString = FilterText.ToString().ToLower();
    if (FilterString.Len() == 0) {
        // No filter specified. Include everything
        FilteredMarkerListEntries = MarkerListEntries;
    }
    else {
        FilteredMarkerListEntries = MarkerListEntries.FilterByPredicate(
            [_FilterString = FilterString](TSharedPtr<FMarkerListEntry> Item) -> bool {
                FString ItemString = Item->MarkerName.ToLower();
                return ItemString.Find(_FilterString, ESearchCase::CaseSensitive) != INDEX_NONE;
            });
    }

    // Filter the results
    MarkersListView->RequestListRefresh();
}

