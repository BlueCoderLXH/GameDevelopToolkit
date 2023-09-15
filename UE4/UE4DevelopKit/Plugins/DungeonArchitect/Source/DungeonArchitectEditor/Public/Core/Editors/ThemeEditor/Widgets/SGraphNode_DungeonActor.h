//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonActorBase.h"

#include "AssetThumbnail.h"
#include "SGraphNode.h"

/**
 * Implements the message interaction graph panel.
 */
class DUNGEONARCHITECTEDITOR_API SGraphNode_DungeonActor : public SGraphNode, public FNodePropertyObserver {
public:
    SLATE_BEGIN_ARGS(SGraphNode_DungeonActor) {}
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, UEdGraphNode_DungeonActorBase* InNode);

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    virtual void CreatePinWidgets() override;
    virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
    virtual TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;
    // End of SGraphNode interface

    // SNodePanel::SNode interface
    virtual void MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter) override;

    // End of SNodePanel::SNode interface

    void RebuildExecutionOrder();

    // FPropertyObserver interface
    virtual void OnPropertyChanged(UEdGraphNode_DungeonBase* Sender, const FName& PropertyName) override;

    // End of FPropertyObserver interface

    struct FObjectOrAssetData {
        UObject* Object;
        FAssetData AssetData;

        FObjectOrAssetData(UObject* InObject = nullptr)
            : Object(InObject) {
            AssetData = InObject != nullptr && !InObject->IsA<AActor>() ? FAssetData(InObject) : FAssetData();
        }

        FObjectOrAssetData(const FAssetData& InAssetData)
            : Object(nullptr)
              , AssetData(InAssetData) {
        }

        bool IsValid() const {
            return Object != nullptr || AssetData.IsValid();
        }
    };

    /** @return The value or unset if properties with multiple values are viewed */
    TOptional<float> OnGetAffinityValue() const;
    void OnAffinityValueChanged(float NewValue);
    void OnAffinityValueCommitted(float NewValue, ETextCommit::Type CommitInfo);

private:
    void SetThumbnail(UObject* AssetObject);
    void BuildThumbnailWidget();
    bool GetValue(FObjectOrAssetData& OutValue) const;

    /** Get the visibility of the index overlay */
    EVisibility GetIndexVisibility() const;

    /** Get the text to display in the index overlay */
    FText GetIndexText() const;

    /** Get the tooltip for the index overlay */
    FText GetIndexTooltipText() const;

    /** Get the color to display for the index overlay. This changes on hover state of sibling nodes */
    FSlateColor GetIndexColor(bool bHovered) const;

    /** Handle hover state changing for the index widget - we use this to highlight sibling nodes */
    void OnIndexHoverStateChanged(bool bHovered);

protected:
    FSlateColor GetBorderBackgroundColor() const;
    virtual const FSlateBrush* GetNameIcon() const;

    UEdGraphNode_DungeonActorBase* EdActorNode;
    FIntPoint ThumbnailSize;

    TSharedPtr<SHorizontalBox> OutputPinBox;
    /** Thumbnail for the asset */
    TSharedPtr<class FAssetThumbnail> AssetThumbnail;

    /** Cached data */
    mutable FAssetData CachedAssetData;

    /** The widget we use to display the index of the node */
    TSharedPtr<SWidget> IndexOverlay;
};

