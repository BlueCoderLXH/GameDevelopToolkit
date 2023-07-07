//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Common/Widgets/SGridFlowItemOverlay.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SGridFlowItemOverlay"


void SGridFlowItemOverlay::Construct(const FArguments& InArgs, const UFlowGraphItem* InItem) {
    Item = InItem;
    Selected = InArgs._Selected;

    // TODO: Move this to DA style sheet
    const FSlateBrush* BodyBrush = FDungeonArchitectStyle::Get().GetBrush(TEXT("FlowEditor.ItemWidget.Body"));
    const FSlateBrush* SelectionBrush = FDungeonArchitectStyle::Get().GetBrush(
        TEXT("FlowEditor.ItemWidget.Body.Selected"));
    FSlateFontInfo Font = FDungeonArchitectStyle::Get().GetFontStyle("FlowEditor.ItemWidget.Text");
    if (Item.IsValid() && Item->CustomInfo.FontScale != 1.0f) {
        Font.Size = FMath::RoundToInt(Font.Size * Item->CustomInfo.FontScale);
    }

    WidgetRadius = BodyBrush->ImageSize.X * 0.5f;
    ChildSlot
    [
        SNew(SOverlay)
        + SOverlay::Slot()
          .HAlign(HAlign_Fill)
          .VAlign(VAlign_Fill)
        [
            // Add a dummy box here to make sure the widget doesn't get smaller than the brush
            SNew(SBox)
			.WidthOverride(BodyBrush->ImageSize.X)
			.HeightOverride(BodyBrush->ImageSize.Y)
        ]
        + SOverlay::Slot()
          .HAlign(HAlign_Fill)
          .VAlign(VAlign_Fill)
        [
            SNew(SBorder)
			.BorderImage(BodyBrush)
			.BorderBackgroundColor(this, &SGridFlowItemOverlay::GetColor)
			.Padding(FMargin(4.0f, 0.0f, 4.0f, 1.0f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
            [
                SNew(STextBlock)
				.Text(this, &SGridFlowItemOverlay::GetItemText)
				.ColorAndOpacity(this, &SGridFlowItemOverlay::GetTextColor)
				.Font(Font)
            ]
        ]
        + SOverlay::Slot()
          .HAlign(HAlign_Fill)
          .VAlign(VAlign_Fill)
        [
            SNew(SBorder)
			.Visibility(this, &SGridFlowItemOverlay::GetSelectionImageVisiblity)
			.BorderImage(SelectionBrush)
        ]

    ];
}

void SGridFlowItemOverlay::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
    bHovered = true;
}

void SGridFlowItemOverlay::OnMouseLeave(const FPointerEvent& MouseEvent) {
    SCompoundWidget::OnMouseLeave(MouseEvent);
    bHovered = false;
}

FReply SGridFlowItemOverlay::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    if (Item.IsValid()) {
        OnMousePressed.ExecuteIfBound(Item->ItemId, false);
    }
    return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SGridFlowItemOverlay::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
    if (Item.IsValid()) {
        OnMousePressed.ExecuteIfBound(Item->ItemId, true);
    }
    return SCompoundWidget::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FSlateColor SGridFlowItemOverlay::GetColor() const {
    return FGridFlowItemVisuals::GetBackgroundColor(Item.Get(), bHovered);
}

FSlateColor SGridFlowItemOverlay::GetTextColor() const {
    return FGridFlowItemVisuals::GetTextColor(Item.Get(), bHovered);
}

FText SGridFlowItemOverlay::GetItemText() const {
    return FText::FromString(FGridFlowItemVisuals::GetText(Item.Get()));
}

void SGridFlowItemOverlay::SetBaseOffset(const FVector2D& InBaseOffset) {
    BaseOffset = InBaseOffset;
}

EVisibility SGridFlowItemOverlay::GetSelectionImageVisiblity() const {
    return Selected.Get() ? EVisibility::Visible : EVisibility::Collapsed;
}


#undef LOCTEXT_NAMESPACE

