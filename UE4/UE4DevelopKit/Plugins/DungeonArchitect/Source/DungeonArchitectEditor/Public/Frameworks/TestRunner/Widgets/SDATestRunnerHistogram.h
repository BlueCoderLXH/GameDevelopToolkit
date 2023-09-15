//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class IDATestRunnerHistogramDataSource {
public:
    virtual ~IDATestRunnerHistogramDataSource() {
    }

    virtual int32 GetNumBars() = 0;
    virtual float GetTotalValue() = 0;
    virtual float GetMinValue() = 0;
    virtual float GetBarValue(int32 Index) = 0;
    virtual FString GetBarText(int32 Index) = 0;
    virtual FLinearColor GetBarColor(int32 Index) = 0;
};

class SDATestRunnerHistogram : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SDATestRunnerHistogram) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<IDATestRunnerHistogramDataSource> InDataSource);
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                      FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
                      bool bParentEnabled) const override;
    virtual FVector2D ComputeDesiredSize(float) const override;

private:
    TSharedPtr<IDATestRunnerHistogramDataSource> DataSource;
};

