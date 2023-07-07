//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "SNodePanel.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SGraphPin;
class UGridFlowAbstractEdGraphNode;
class SGridFlowItemOverlay;

class SGraphNode_GridFlowAbstractNode : public SGraphNode {
public:
    SLATE_BEGIN_ARGS(SGraphNode_GridFlowAbstractNode) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UGridFlowAbstractEdGraphNode* InNode);

    struct FLinkItemWidgetInfo {
        TSharedPtr<SGridFlowItemOverlay> ItemWidget;
        FGuid DestinationNodeId;
    };

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    virtual void CreatePinWidgets() override;
    virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
    virtual const FSlateBrush* GetShadowBrush(bool bSelected) const override;
    virtual TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;
    virtual FVector2D ComputeDesiredSize(float) const override;
    // End of SGraphNode interface

    TArray<TSharedPtr<SGridFlowItemOverlay>> GetNodeItemWidgets() const { return NodeItemWidgets; }
    TArray<FLinkItemWidgetInfo> GetLinkItemWidgets() const { return LinkItemWidgets; }

protected:
    static FText GetPreviewCornerText();
    FSlateColor GetNodeColor() const;
    FSlateColor GetTextColor() const;
    FLinearColor GetTextShadowColor() const;
    void CreateNodeItemWidgets();
    void CreateLinkItemWidgets();
    void OnItemClicked(const FGuid& InItemId, bool bDoubleClicked);
    bool IsItemSelected(FGuid InItemId) const;

protected:
    TArray<TSharedPtr<SGridFlowItemOverlay>> NodeItemWidgets;
    TArray<FLinkItemWidgetInfo> LinkItemWidgets;
};

