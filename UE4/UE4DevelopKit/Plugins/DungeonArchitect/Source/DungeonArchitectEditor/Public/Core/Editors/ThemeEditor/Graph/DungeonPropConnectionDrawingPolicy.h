//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"

// This class draws the connections for an UEdGraph with a behavior tree schema
class DUNGEONARCHITECTEDITOR_API FDungeonPropConnectionDrawingPolicy : public FConnectionDrawingPolicy {
protected:
    UEdGraph* GraphObj;

    TMap<UEdGraphNode*, int32> NodeWidgetMap;
public:
    //
    FDungeonPropConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor,
                                        const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements,
                                        UEdGraph* InGraphObj);

    // FConnectionDrawingPolicy interface 
    virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/
                                      FConnectionParams& Params) override;
    virtual void Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& PinGeometries, FArrangedChildren& ArrangedNodes) override;
    virtual void DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom,
                                     const FConnectionParams& Params) override;
    virtual void DrawSplineWithArrow(const FVector2D& StartPoint, const FVector2D& EndPoint,
                                     const FConnectionParams& Params) override;
    virtual void DrawPreviewConnector(const FGeometry& PinGeometry, const FVector2D& StartPoint, const FVector2D& EndPoint,
                                      UEdGraphPin* Pin) override;
    virtual void DrawConnection(int32 LayerId, const FVector2D& Start, const FVector2D& End,
                                const FConnectionParams& Params) override;
    // End of FConnectionDrawingPolicy interface

protected:
    void Internal_DrawLineWithArrow(const FVector2D& StartAnchorPoint, const FVector2D& EndAnchorPoint,
                                    const FConnectionParams& Params);
};

