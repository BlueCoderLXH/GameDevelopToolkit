//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractConnectionDrawingPolicy.h"

#include "Frameworks/Flow/Common/Widgets/SGridFlowItemOverlay.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractLink.h"
#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractEdGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodeBase.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodes.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Widgets/SGraphNode_GridFlowAbstractNode.h"

#include "EdGraph/EdGraphNode.h"
#include "Rendering/DrawElements.h"
#include "SGraphNode.h"
#include "ToolMenu.h"

#define LOCTEXT_NAMESPACE "GrammarGraphConnectionDrawingPolicy"

FGridFlowAbstractConnectionDrawingPolicy::FGridFlowAbstractConnectionDrawingPolicy(
    int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect,
    FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
    : FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
      , GraphObj(InGraphObj) {
}

void FGridFlowAbstractConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
                                                                    /*inout*/ FConnectionParams& Params) {
    const FLinearColor DefaultColor(1.0f, 1.0f, 1.0f);
    Params.WireThickness = 1.5f;
    Params.WireColor = DefaultColor;

    bool bConnected = true;
    bool bOneWay = false;

    if (OutputPin && InputPin) {
        UGridFlowAbstractEdGraphNodeBase* SourceNode = Cast<UGridFlowAbstractEdGraphNodeBase>(
            OutputPin->GetOwningNode());
        UGridFlowAbstractEdGraphNodeBase* DestNode = Cast<UGridFlowAbstractEdGraphNodeBase>(InputPin->GetOwningNode());
        if (SourceNode && DestNode) {
            UGridFlowAbstractEdGraph* AbstractEdGraph = Cast<UGridFlowAbstractEdGraph>(SourceNode->GetGraph());
            if (AbstractEdGraph && AbstractEdGraph->ScriptGraph.IsValid()) {
                UGridFlowAbstractGraph* ScriptGraph = AbstractEdGraph->ScriptGraph.Get();
                UFlowAbstractLink* ScriptLink = ScriptGraph->GetLink(SourceNode->NodeGuid, DestNode->NodeGuid);
                if (ScriptLink) {
                    if (ScriptLink->Type == EFlowAbstractLinkType::Unconnected) {
                        bConnected = false; // Set as not connected
                    }
                    else if (ScriptLink->Type == EFlowAbstractLinkType::OneWay) {
                        bOneWay = true; // Set as One Way
                    }
                }
            }
        }
    }

    // User Flag Settings:
    // bUserFlag1: False = Unconnected, True = Connected
    // bUserFlag2: True = OnWay
    Params.bUserFlag1 = bConnected;
    Params.bUserFlag2 = bOneWay;

    if (!bConnected) {
        Params.WireColor = FLinearColor(0, 0, 0, 0.75f);
    }
    else if (bOneWay) {
        Params.WireColor = FLinearColor(1, 0.25f, 0);
    }


    const bool bDeemphasizeUnhoveredPins = HoveredPins.Num() > 0;
    if (bDeemphasizeUnhoveredPins) {
        ApplyHoverDeemphasis(OutputPin, InputPin, /*inout*/ Params.WireThickness, /*inout*/ Params.WireColor);
    }
}

void FGridFlowAbstractConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
                                                    FArrangedChildren& ArrangedNodes) {
    // Build an acceleration structure to quickly find geometry for the nodes
    NodeWidgetMap.Empty();
    for (int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex) {
        FArrangedWidget& CurWidget = ArrangedNodes[NodeIndex];
        TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);
        UEdGraphNode* NodeObj = ChildNode->GetNodeObj();
        if (NodeObj) {
            NodeWidgetMap.Add(NodeObj->NodeGuid, NodeIndex);
        }
    }

    // Now draw
    FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);

    static const FVector2D NodeSize = FVector2D(64, 64);
    FVector2D DrawScale = FVector2D(1, 1);

    // Build the item widget info
    ItemWidgetMap.Empty();
    for (int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex) {
        FArrangedWidget& CurWidget = ArrangedNodes[NodeIndex];
        DrawScale = CurWidget.Geometry.GetDrawSize() / NodeSize;
        TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);
        UEdGraphNode* NodeObj = ChildNode->GetNodeObj();
        if (NodeObj->IsA<UGridFlowAbstractEdGraphNode>()) {
            TSharedRef<SGraphNode_GridFlowAbstractNode> HostWidget = StaticCastSharedRef<SGraphNode_GridFlowAbstractNode
            >(CurWidget.Widget);
            TArray<TSharedPtr<SGridFlowItemOverlay>> NodeItemWidgets = HostWidget->GetNodeItemWidgets();
            for (TSharedPtr<SGridFlowItemOverlay> ItemWidget : NodeItemWidgets) {
                const UFlowGraphItem* Item = ItemWidget->GetItem();
                if (!Item) continue;
                FVector2D HostCenter = FGeometryHelper::CenterOf(CurWidget.Geometry);
                FVector2D ItemOffset = ItemWidget->GetBaseOffset();
                FVector2D ItemCenter = HostCenter + ItemOffset * DrawScale;

                FItemWidgetInfo& ItemInfo = ItemWidgetMap.FindOrAdd(Item->ItemId);
                ItemInfo.ItemWidget = ItemWidget;
                ItemInfo.ItemCenter = ItemCenter;
                ItemInfo.ItemRadius = ItemWidget->GetWidgetRadius();
            }


            TArray<SGraphNode_GridFlowAbstractNode::FLinkItemWidgetInfo> LinkItemWidgets = HostWidget->GetLinkItemWidgets();
            for (const SGraphNode_GridFlowAbstractNode::FLinkItemWidgetInfo& LinkItemInfo : LinkItemWidgets) {
                int32* DestNodeIdxPtr = NodeWidgetMap.Find(LinkItemInfo.DestinationNodeId);
                if (DestNodeIdxPtr) {
                    int32 DestNodeIndex = *DestNodeIdxPtr;
                    FArrangedWidget& DstWidget = ArrangedNodes[DestNodeIndex];

                    const UFlowGraphItem* Item = LinkItemInfo.ItemWidget->GetItem();
                    if (!Item) continue;
                    
                    FVector2D SrcCenter = FGeometryHelper::CenterOf(CurWidget.Geometry);
                    FVector2D DstCenter = FGeometryHelper::CenterOf(DstWidget.Geometry);

                    FVector2D ItemOffset = LinkItemInfo.ItemWidget->GetBaseOffset();
                    FVector2D ItemCenter = SrcCenter + (DstCenter - SrcCenter) * 0.5f; //HostCenter + ItemOffset;

                    FItemWidgetInfo& ItemInfo = ItemWidgetMap.FindOrAdd(Item->ItemId);
                    ItemInfo.ItemWidget = LinkItemInfo.ItemWidget;
                    ItemInfo.ItemCenter = ItemCenter;
                    ItemInfo.ItemRadius = LinkItemInfo.ItemWidget->GetWidgetRadius();
                }
            }
        }
    }

    // Draw the item references
    for (auto& Entry : ItemWidgetMap) {
        FGuid& ItemId = Entry.Key;
        const FItemWidgetInfo& ItemInfo = Entry.Value;
        TSharedPtr<SGridFlowItemOverlay> ItemWidget = ItemInfo.ItemWidget.Pin();
        if (ItemWidget.IsValid()) {
            const UFlowGraphItem* Item = ItemWidget->GetItem();
            FLinearColor Color = FLinearColor::Red;
            if (Item->ItemType == EFlowGraphItemType::Teleporter) {
                Color = FLinearColor(0, 1, 1);
            }
            
            for (FGuid RefItemId : Item->ReferencedItemIds) {
                FItemWidgetInfo* RefItemPtr = ItemWidgetMap.Find(RefItemId);
                if (RefItemPtr) {
                    FVector2D SrcItemCenter = ItemInfo.ItemCenter;
                    FVector2D DstItemCenter = RefItemPtr->ItemCenter;
                    FVector2D Direction = DstItemCenter - SrcItemCenter;
                    Direction.Normalize();

                    FVector2D Start = SrcItemCenter + Direction * ItemInfo.ItemRadius * DrawScale;
                    FVector2D End = DstItemCenter - Direction * RefItemPtr->ItemRadius * DrawScale;

                    FConnectionParams Params;
                    Params.WireThickness = 1.0f;
                    Params.WireColor = Color;
                    Params.bUserFlag1 = true; //  Is Connected
                    Params.bUserFlag2 = false; // Is Not OneWay
                    Internal_DrawLineWithArrow(Start, End, Params, true);
                }
            }
        }
    }
}

void FGridFlowAbstractConnectionDrawingPolicy::DetermineLinkGeometry(FArrangedChildren& ArrangedNodes,
                                                                     TSharedRef<SWidget>& OutputPinWidget,
                                                                     UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
                                                                     /*out*/ FArrangedWidget*& StartWidgetGeometry,
                                                                     /*out*/ FArrangedWidget*& EndWidgetGeometry) {
    UEdGraphNode* OutputNode = OutputPin->GetOwningNode();
    UEdGraphNode* InputNode = InputPin->GetOwningNode();

    if (OutputNode && NodeWidgetMap.Contains(OutputNode->NodeGuid)) {
        StartWidgetGeometry = &ArrangedNodes[NodeWidgetMap[OutputNode->NodeGuid]];
    }
    if (InputNode && NodeWidgetMap.Contains(InputNode->NodeGuid)) {
        EndWidgetGeometry = &ArrangedNodes[NodeWidgetMap[InputNode->NodeGuid]];
    }
}

void FGridFlowAbstractConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry,
                                                                    const FVector2D& StartPoint,
                                                                    const FVector2D& EndPoint, UEdGraphPin* Pin) {
    FConnectionParams Params;
    Params.WireThickness = 1.0f;
    Params.WireColor = FLinearColor::White;
    Params.bDrawBubbles = false;
    DetermineWiringStyle(Pin, nullptr, /*inout*/ Params);

    if (Pin->Direction == EGPD_Output) {
        DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, EndPoint), EndPoint, Params);
    }
    else {
        DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, StartPoint), StartPoint, Params);
    }
}


void FGridFlowAbstractConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D& StartAnchorPoint,
                                                                   const FVector2D& EndAnchorPoint,
                                                                   const FConnectionParams& Params) {
    Internal_DrawLineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

void FGridFlowAbstractConnectionDrawingPolicy::Internal_DrawLineWithArrow(
    const FVector2D& StartAnchorPoint, const FVector2D& EndAnchorPoint, const FConnectionParams& Params,
    bool bForceForeground) {
    const FVector2D ArrowHeadRadius = ArrowRadius;
    const FSlateBrush* ArrowHeadImage = ArrowImage;

    const FVector2D DeltaPos = EndAnchorPoint - StartAnchorPoint;
    const FVector2D UnitDelta = DeltaPos.GetSafeNormal();
    const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

    // Come up with the final start/end points
    const FVector2D LengthBias = ArrowHeadRadius.X * UnitDelta;

    const FVector2D StartPoint = StartAnchorPoint;
    FVector2D EndPoint = EndAnchorPoint;

    bool bConnected = Params.bUserFlag1;
    bool bOneWay = Params.bUserFlag2;

    if (bConnected) {
        EndPoint -= LengthBias;
    }
    if (bOneWay) {
        EndPoint -= LengthBias;
    }

    int32 LineLayer = bForceForeground ? ArrowLayerID : WireLayerID;
    DrawConnection(LineLayer, StartPoint, EndPoint, Params);

    FVector2D ArrowDrawPos = (EndAnchorPoint - LengthBias) - ArrowHeadRadius;
    const float AngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

    if (bConnected) {
        // Draw the arrow
        FSlateDrawElement::MakeRotatedBox(
            DrawElementsList,
            ArrowLayerID,
            FPaintGeometry(ArrowDrawPos, ArrowHeadImage->ImageSize * ZoomFactor, ZoomFactor),
            ArrowHeadImage,
            //ClippingRect,
            ESlateDrawEffect::None,
            AngleInRadians,
            TOptional<FVector2D>(),
            FSlateDrawElement::RelativeToElement,
            Params.WireColor
        );
    }
    if (bOneWay) {
        // Draw another arrow head
        ArrowDrawPos = (EndAnchorPoint - LengthBias * 2) - ArrowHeadRadius;
        FSlateDrawElement::MakeRotatedBox(
            DrawElementsList,
            ArrowLayerID,
            FPaintGeometry(ArrowDrawPos, ArrowHeadImage->ImageSize * ZoomFactor, ZoomFactor),
            ArrowHeadImage,
            //ClippingRect,
            ESlateDrawEffect::None,
            AngleInRadians,
            TOptional<FVector2D>(),
            FSlateDrawElement::RelativeToElement,
            Params.WireColor
        );
    }

}

void FGridFlowAbstractConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom,
                                                                   const FConnectionParams& Params) {
    // Get a reasonable seed point (halfway between the boxes)
    FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
    FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);

    float StartRadius = StartGeom.GetDrawSize().X * 0.5f;
    float EndRadius = EndGeom.GetDrawSize().X * 0.5f;

    const float FallbackRadius = 32;
    if (StartRadius == 0) {
        StartRadius = FallbackRadius;
        StartCenter += FVector2D(StartRadius, StartRadius);
    }
    if (EndRadius == 0) {
        EndRadius = FallbackRadius;
        EndCenter += FVector2D(EndRadius, EndRadius);
    }

    FVector2D Direction = (EndCenter - StartCenter);
    Direction.Normalize();

    const FVector2D StartAnchorPoint = StartCenter + Direction * StartRadius;
    const FVector2D EndAnchorPoint = EndCenter - Direction * EndRadius;

    DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

void FGridFlowAbstractConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FVector2D& Start,
                                                              const FVector2D& End, const FConnectionParams& Params) {
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

}


#undef LOCTEXT_NAMESPACE

