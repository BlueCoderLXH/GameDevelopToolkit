//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/TestRunner/Widgets/SDATestRunnerHistogram.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#include "EditorStyleSet.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SOverlay.h"

void SDATestRunnerHistogram::Construct(const FArguments& InArgs,
                                       TSharedPtr<IDATestRunnerHistogramDataSource> InDataSource) {
    DataSource = InDataSource;

    ChildSlot
    [
        SNew(SOverlay)
        .Visibility(EVisibility::SelfHitTestInvisible)
    ];
}

int32 SDATestRunnerHistogram::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                      const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                      int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
    const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get()
                                                             .GetRenderer()->GetFontMeasureService();

    // Rendering info.
    const bool bEnabled = ShouldBeEnabled(bParentEnabled);
    const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
    const FSlateBrush* TimelineAreaBrush = FEditorStyle::GetBrush("Profiler.LineGraphArea");
    const FSlateBrush* FillImage = FDungeonArchitectStyle::Get().GetBrush("FlowEditor.TestRunner.BarBody");

    // Draw background.
    FSlateDrawElement::MakeBox
    (
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(FVector2D(0, 0),
                                         FVector2D(AllottedGeometry.GetLocalSize().X,
                                                   AllottedGeometry.GetLocalSize().Y)),
        TimelineAreaBrush,
        DrawEffects,
        TimelineAreaBrush->GetTint(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint()
    );
    LayerId++;

    const float LabelBuffer = 25.0f;

    const int32 NumBars = DataSource->GetNumBars();
    const float MinValue = DataSource->GetMinValue();
    float TotalCount = DataSource->GetTotalValue();
    if (FMath::Fmod(TotalCount, 4) != 0) {
        TotalCount += 4 - FMath::Fmod(TotalCount, 4);
    }

    // draw the grid lines
    const uint32 CountX = static_cast<uint32>((AllottedGeometry.Size.X - LabelBuffer * 2.0f) / NumBars);
    const float StartX = LabelBuffer;
    static const FLinearColor GridColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.25f);
    static const FLinearColor GridTextColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.25f);
    static const FLinearColor BorderColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
    const FSlateFontInfo SummaryFont = FCoreStyle::GetDefaultFontStyle("Regular", 8);
    const float MaxFontCharHeight = FontMeasureService->Measure(TEXT("!"), SummaryFont).Y;
    TArray<FVector2D> LinePoints;

    // draw the histogram box
    LinePoints.Add(FVector2D(StartX - 1, LabelBuffer - 1));
    LinePoints.Add(FVector2D(StartX + NumBars * CountX + 1, LabelBuffer - 1));
    LinePoints.Add(FVector2D(StartX + NumBars * CountX + 1, AllottedGeometry.GetLocalSize().Y - LabelBuffer + 1));
    LinePoints.Add(FVector2D(StartX - 1, AllottedGeometry.Size.Y - LabelBuffer + 1));
    LinePoints.Add(FVector2D(StartX - 1, LabelBuffer - 1));
    FSlateDrawElement::MakeLines(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(),
        LinePoints,
        DrawEffects,
        BorderColor
    );
    LinePoints.Empty();
    LayerId++;


    // draw the vertical lines
    for (int32 Index = 0; Index < NumBars; ++Index) {
        const float MarkerPosX = StartX + Index * CountX;
        const float MarkerTextPosX = StartX + (Index + 0.5f) * CountX;
        LinePoints.Add(FVector2D(MarkerPosX, LabelBuffer - 1));
        LinePoints.Add(FVector2D(MarkerPosX, AllottedGeometry.GetLocalSize().Y - LabelBuffer + 1));
        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            LinePoints,
            DrawEffects,
            GridColor
        );
        LinePoints.Empty();

        // Bottom - X-Axes numbers, starting from MinValue
        //const FString XLabel = FString::Printf(TEXT("%.0f"), MinValue + Index * Interval);
        const FString XLabel = DataSource->GetBarText(Index);
        const float FontCharWidth = FontMeasureService->Measure(XLabel, SummaryFont).X;
        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToOffsetPaintGeometry(FVector2D(MarkerTextPosX - FontCharWidth / 2.0f,
                                                             AllottedGeometry.GetLocalSize().Y - LabelBuffer / 2.0f -
                                                             MaxFontCharHeight / 2.0f)),
            XLabel,
            SummaryFont,
            DrawEffects,
            FLinearColor::White
        );

    }
    LayerId++;

    // draw the horizontal lines
    const float CountY = (AllottedGeometry.GetLocalSize().Y - LabelBuffer * 2.0f) / 4;
    const float StartY = LabelBuffer;
    for (int32 Index = 0; Index < 5; ++Index) {
        const float MarkerPosY = StartY + Index * CountY;
        LinePoints.Add(FVector2D(StartX, MarkerPosY));
        LinePoints.Add(FVector2D(StartX + NumBars * CountX, MarkerPosY));
        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            LinePoints,
            DrawEffects,
            GridColor
        );
        LinePoints.Empty();

        // Bottom - Y-Axes numbers, starting from 0
        const int32 YLabelInt = FMath::RoundToInt(TotalCount / 4 * (4 - Index));
        const FString YLabel = FString::FromInt(YLabelInt);
        const float FontCharWidth = FontMeasureService->Measure(YLabel, SummaryFont).X;
        FSlateDrawElement::MakeText
        (
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToOffsetPaintGeometry(FVector2D(LabelBuffer / 2.0f - FontCharWidth / 2.0f,
                                                             MarkerPosY - MaxFontCharHeight / 2.0f)),
            YLabel,
            SummaryFont,
            DrawEffects,
            FLinearColor::White
        );

    }
    LayerId++;

    for (int32 Index = 0; Index < NumBars; ++Index) {
        const float BarWidth = 0.75f;
        const float MarkerPosX = StartX + (Index + (1.0f - BarWidth) * 0.5f) * CountX;
        const float CurrentCount = DataSource->GetBarValue(Index);
        FLinearColor BarColor = DataSource->GetBarColor(Index);
        const float SizeY = static_cast<float>(CurrentCount) / static_cast<float>(TotalCount) * (AllottedGeometry
                                                                                                 .GetLocalSize().Y -
            LabelBuffer * 2.0f);
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(
                FVector2D(MarkerPosX, AllottedGeometry.GetLocalSize().Y - SizeY - LabelBuffer),
                FVector2D(CountX * BarWidth, SizeY)),
            FillImage,
            DrawEffects,
            BarColor
        );
    }
    return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
                                    bParentEnabled && IsEnabled());
}

FVector2D SDATestRunnerHistogram::ComputeDesiredSize(float) const {
    //return SCompoundWidget::ComputeDesiredSize();
    return FVector2D(16.0f, 16.0f);
}

