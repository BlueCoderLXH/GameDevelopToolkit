//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/DungeonPropConnectionDrawingPolicy.h"


#define LOCTEXT_NAMESPACE "DungeonPropConnectionDrawingPolicy"


FDungeonPropConnectionDrawingPolicy::FDungeonPropConnectionDrawingPolicy(
    int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect,
    FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
    : FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
      , GraphObj(InGraphObj) {
}

void FDungeonPropConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/
                                                               FConnectionParams& Params) {
    const FLinearColor DefaultColor(1.0f, 1.0f, 1.0f);
    Params.WireThickness = 1.5f;
    Params.WireColor = DefaultColor;
    Params.bUserFlag1 = false; // bidirectional

    const bool bDeemphasizeUnhoveredPins = HoveredPins.Num() > 0;
    if (bDeemphasizeUnhoveredPins) {
        ApplyHoverDeemphasis(OutputPin, InputPin, /*inout*/ Params.WireThickness, /*inout*/ Params.WireColor);
    }
}

void FDungeonPropConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
                                               FArrangedChildren& ArrangedNodes) {
    // Build an acceleration structure to quickly find geometry for the nodes
    NodeWidgetMap.Empty();
    for (int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex) {
        FArrangedWidget& CurWidget = ArrangedNodes[NodeIndex];
        TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);
        NodeWidgetMap.Add(ChildNode->GetNodeObj(), NodeIndex);
    }

    // Now draw
    FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

void FDungeonPropConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry,
                                                               const FVector2D& StartPoint, const FVector2D& EndPoint,
                                                               UEdGraphPin* Pin) {
    bool bBiDirectional = false;
    FConnectionParams Params;
    Params.WireThickness = 1.0f;
    Params.WireColor = FLinearColor::White;
    Params.bDrawBubbles = false;
    Params.bUserFlag1 = bBiDirectional;
    DetermineWiringStyle(Pin, nullptr, /*inout*/ Params);

    if (Pin->Direction == EGPD_Output) {
        DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, EndPoint), EndPoint, Params);
    }
    else {
        DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, StartPoint), StartPoint, Params);
    }

}


void FDungeonPropConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D& StartAnchorPoint,
                                                              const FVector2D& EndAnchorPoint,
                                                              const FConnectionParams& Params) {
    // hacky: use bBidirectional flag to reverse direction of connection (used by debugger)
    bool Bidirectional = Params.bUserFlag1;
    const FVector2D& P0 = Bidirectional ? EndAnchorPoint : StartAnchorPoint;
    const FVector2D& P1 = Bidirectional ? StartAnchorPoint : EndAnchorPoint;

    Internal_DrawLineWithArrow(P0, P1, Params);
}

void FDungeonPropConnectionDrawingPolicy::Internal_DrawLineWithArrow(const FVector2D& StartAnchorPoint,
                                                                     const FVector2D& EndAnchorPoint,
                                                                     const FConnectionParams& Params) {
    //@TODO: Should this be scaled by zoom factor?
    const float LineSeparationAmount = 4.5f;

    const FVector2D DeltaPos = EndAnchorPoint - StartAnchorPoint;
    const FVector2D UnitDelta = DeltaPos.GetSafeNormal();
    const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

    // Come up with the final start/end points
    const FVector2D DirectionBias = Normal * LineSeparationAmount;
    const FVector2D LengthBias = ArrowRadius.X * UnitDelta;
    const FVector2D StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
    const FVector2D EndPoint = EndAnchorPoint + DirectionBias - LengthBias;

    // Draw a line/spline
    DrawConnection(WireLayerID, StartPoint, EndPoint, Params);

    // Draw the arrow
    const FVector2D ArrowDrawPos = EndPoint - ArrowRadius;
    const float AngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

    FSlateDrawElement::MakeRotatedBox(
        DrawElementsList,
        ArrowLayerID,
        FPaintGeometry(ArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
        ArrowImage,
        //ClippingRect,
        ESlateDrawEffect::None,
        AngleInRadians,
        TOptional<FVector2D>(),
        FSlateDrawElement::RelativeToElement,
        Params.WireColor
    );
}

void FDungeonPropConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom,
                                                              const FConnectionParams& Params) {
    // Get a reasonable seed point (halfway between the boxes)
    const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
    const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
    const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

    // Find the (approximate) closest points between the two boxes
    const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
    const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

    DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

void FDungeonPropConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FVector2D& Start, const FVector2D& End,
                                                         const FConnectionParams& Params) {
    const FVector2D& P0 = Start;
    const FVector2D& P1 = End;

    const FVector2D Delta = End - Start;
    const FVector2D NormDelta = Delta.GetSafeNormal();

    const FVector2D P0Tangent = NormDelta;
    const FVector2D P1Tangent = NormDelta;

    // Draw the spline itself
    FSlateDrawElement::MakeDrawSpaceSpline(
        DrawElementsList,
        LayerId,
        P0, P0Tangent,
        P1, P1Tangent,
        //ClippingRect,
        Params.WireThickness,
        ESlateDrawEffect::None,
        Params.WireColor
    );

    if (Params.bDrawBubbles) {
        // This table maps distance along curve to alpha
        FInterpCurve<float> SplineReparamTable;
        float SplineLength = MakeSplineReparamTable(P0, P0Tangent, P1, P1Tangent, SplineReparamTable);

        // Draw bubbles on the spline
        const float BubbleSpacing = 64.f * ZoomFactor;
        const float BubbleSpeed = 192.f * ZoomFactor;
        const FVector2D BubbleSize = BubbleImage->ImageSize * ZoomFactor * 0.1f * Params.WireThickness;

        float Time = (FPlatformTime::Seconds() - GStartTime);
        const float BubbleOffset = FMath::Fmod(Time * BubbleSpeed, BubbleSpacing);
        const int32 NumBubbles = FMath::CeilToInt(SplineLength / BubbleSpacing);
        for (int32 i = 0; i < NumBubbles; ++i) {
            const float Distance = (static_cast<float>(i) * BubbleSpacing) + BubbleOffset;
            if (Distance < SplineLength) {
                const float Alpha = SplineReparamTable.Eval(Distance, 0.f);
                FVector2D BubblePos = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, Alpha);
                BubblePos -= (BubbleSize * 0.5f);

                FSlateDrawElement::MakeBox(
                    DrawElementsList,
                    LayerId,
                    FPaintGeometry(BubblePos, BubbleSize, ZoomFactor),
                    BubbleImage,
                    //ClippingRect,
                    ESlateDrawEffect::None,
                    Params.WireColor
                );
            }
        }
    }
}


#undef LOCTEXT_NAMESPACE

