//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadData.h"
#include "Core/Editors/LaunchPad/Widgets/SLaunchPadPage.h"

/////////////// Page - Card Grid /////////////// 
class SLaunchPadPage_CardGrid : public SLaunchPadPage {
public:
    SLATE_BEGIN_ARGS(SLaunchPadPage_CardGrid) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& Args, const FLaunchPadPageLayout_CardGrid& InData);
    virtual FString GetTitle() override;
private:
    FLaunchPadPageLayout_CardGrid Data;
};

struct FLaunchPadPage_CardGridItemSettings {
    int32 CardWidth;
    int32 CardHeight;
    int32 CardPadding;
    int32 ImageWidth;
    int32 ImageHeight;
    bool bShowCategoryTitles;
};

class SLaunchPadPage_CardGridCategory : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SLaunchPadPage_CardGridCategory) {}
        SLATE_ARGUMENT(FLaunchPadPage_CardGridItemSettings, CardSettings)
        SLATE_ARGUMENT(TWeakPtr<SLaunchPadPage_CardGrid>, Parent)
    SLATE_END_ARGS()

    void Construct(const FArguments& Args, const FLaunchPadPageLayout_CardGrid_Category& InCategoryData);

private:
    TSharedRef<class ITableRow> GenerateCardWidget(TSharedPtr<FLaunchPadPageLayout_CardGrid_CardInfo> InItem,
                                                   const TSharedRef<class STableViewBase>& OwnerTable);
    void OnMouseButtonClicked(TSharedPtr<FLaunchPadPageLayout_CardGrid_CardInfo> InCard);

private:
    FLaunchPadPage_CardGridItemSettings CardSettings;
    TWeakPtr<SLaunchPadPage_CardGrid> Parent;

    FLaunchPadPageLayout_CardGrid_Category CategoryData;
    TArray<TSharedPtr<FLaunchPadPageLayout_CardGrid_CardInfo>> CardItems;
};


class SLaunchPadPage_CardGridItemWidget : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SLaunchPadPage_CardGridItemWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& Args, const FLaunchPadPage_CardGridItemSettings& InCardSettings,
                   TSharedPtr<FLaunchPadPageLayout_CardGrid_CardInfo> InItem);
    virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

private:
    bool bIsHyperlink = false;
};

