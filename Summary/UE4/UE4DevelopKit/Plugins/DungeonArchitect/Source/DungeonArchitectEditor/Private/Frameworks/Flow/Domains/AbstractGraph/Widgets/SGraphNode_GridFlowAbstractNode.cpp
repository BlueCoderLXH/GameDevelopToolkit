//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Widgets/SGraphNode_GridFlowAbstractNode.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Frameworks/Flow/Common/Widgets/SGridFlowItemOverlay.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractEdGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodes.h"

#include "SGraphPin.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"

/////////////////////////////////////////////////////
// SGraphNode_GridFlowAbstractNode

void SGraphNode_GridFlowAbstractNode::Construct(const FArguments& InArgs, UGridFlowAbstractEdGraphNode* InNode) {
    this->GraphNode = InNode;

    this->SetCursor(EMouseCursor::CardinalCross);

    this->UpdateGraphNode();
}

void SGraphNode_GridFlowAbstractNode::UpdateGraphNode() {
    InputPins.Empty();
    OutputPins.Empty();

    // Reset variables that are going to be exposed, in case we are refreshing an already setup node.
    RightNodeBox.Reset();
    LeftNodeBox.Reset();

    FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
    TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

    this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
    this->GetOrAddSlot(ENodeZone::Center)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
    [
        SNew(SOverlay)
        // Size Override
        + SOverlay::Slot()
          .HAlign(HAlign_Center)
          .VAlign(VAlign_Center)
        [
            SNew(SBox)
				.WidthOverride(64)
				.HeightOverride(64)
        ]

        // Pin Area
        + SOverlay::Slot()
          .HAlign(HAlign_Fill)
          .VAlign(VAlign_Fill)
        [
            SAssignNew(RightNodeBox, SVerticalBox)
        ]

        // Body
        + SOverlay::Slot()
          .HAlign(HAlign_Fill)
          .VAlign(VAlign_Fill)
          .Padding(0.0f)
        [
            SNew(SBorder)
				.Padding(0.0f)
				.BorderImage(FDungeonArchitectStyle::Get().GetBrush(TEXT("FlowEditor.AbstractNode.Body")))
				.BorderBackgroundColor(this, &SGraphNode_GridFlowAbstractNode::GetNodeColor)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Visibility(EVisibility::Visible)
        ]

        // Text
        + SOverlay::Slot()
          .HAlign(HAlign_Center)
          .VAlign(VAlign_Center)
          .Padding(0.0f)
        [
            SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.Text(this, &SGraphNode_GridFlowAbstractNode::GetEditableNodeTitleAsText)
				.ColorAndOpacity(this, &SGraphNode_GridFlowAbstractNode::GetTextColor)
				.ShadowColorAndOpacity(this, &SGraphNode_GridFlowAbstractNode::GetTextShadowColor)
				.TextStyle(FEditorStyle::Get(), "Graph.StateNode.NodeTitle")
        ]
    ];

    CreatePinWidgets();
    CreateNodeItemWidgets();
    CreateLinkItemWidgets();
}

class SGridFlowAbstractResultNodeOutputPin : public SGraphPin {
public:
    SLATE_BEGIN_ARGS(SGridFlowAbstractResultNodeOutputPin) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin) {
        this->SetCursor(EMouseCursor::Default);

        typedef SGridFlowAbstractResultNodeOutputPin ThisClass;

        bShowLabel = true;

        GraphPinObj = InPin;
        check(GraphPinObj != NULL);

        const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
        check(Schema);

        // Set up a hover for pins that is tinted the color of the pin.
        SBorder::Construct(SBorder::FArguments()
                           .BorderImage(this, &ThisClass::GetPinBorder)
                           .BorderBackgroundColor(this, &ThisClass::GetPinColor)
                           .OnMouseButtonDown(this, &ThisClass::OnPinMouseDown)
                           .Cursor(this, &ThisClass::GetPinCursor)
        );
    }

protected:
    // Begin SGraphPin interface
    virtual TSharedRef<SWidget> GetDefaultValueWidget() override {
        return SNew(STextBlock);
    }

    // End SGraphPin interface

    const FSlateBrush* GetPinBorder() const {
        return (IsHovered())
                   ? FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.BackgroundHovered"))
                   : FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.Background"));
    }
};

void SGraphNode_GridFlowAbstractNode::CreatePinWidgets() {
    UGridFlowAbstractEdGraphNode* ResultNode = CastChecked<UGridFlowAbstractEdGraphNode>(GraphNode);

    UEdGraphPin* CurPin = ResultNode->GetOutputPin();
    TSharedPtr<SGraphPin> NewPin = SNew(SGridFlowAbstractResultNodeOutputPin, CurPin);

    this->AddPin(NewPin.ToSharedRef());
}

void SGraphNode_GridFlowAbstractNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd) {
    PinToAdd->SetOwner(SharedThis(this));
    RightNodeBox->AddSlot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Fill)
                .FillHeight(1.0f)
    [
        PinToAdd
    ];
    OutputPins.Add(PinToAdd);
}

const FSlateBrush* SGraphNode_GridFlowAbstractNode::GetShadowBrush(bool bSelected) const {
    return bSelected
               ? FDungeonArchitectStyle::Get().GetBrush(TEXT("FlowEditor.AbstractNode.Body.Selected"))
               : FDungeonArchitectStyle::Get().GetBrush(TEXT("FlowEditor.AbstractNode.Body.Shadow"));
}

FText SGraphNode_GridFlowAbstractNode::GetPreviewCornerText() {
    return NSLOCTEXT("SGraphNodeAnimStateEntry", "CornerTextDescription", "Execution Graph Result");
}

FSlateColor SGraphNode_GridFlowAbstractNode::GetNodeColor() const {
    UGridFlowAbstractEdGraphNode* EdNode = Cast<UGridFlowAbstractEdGraphNode>(GetNodeObj());
    if (EdNode) {
        UFlowAbstractNode* Node = EdNode->ScriptNode.Get();
        if (Node && Node->bActive) {
            return Node->Color;
        }
    }
    return FLinearColor(0.05f, 0.05f, 0.05f);
}

FSlateColor SGraphNode_GridFlowAbstractNode::GetTextColor() const {
    const FLinearColor NodeColor = GetNodeColor().GetSpecifiedColor();
    const FLinearColor NodeColorHSV = NodeColor.LinearRGBToHSV();
    const float NodeColorV = NodeColorHSV.B;

    FLinearColor TextColor = (NodeColorV > 0.5f) ? FLinearColor::Black : FLinearColor::White;
    TextColor.A = 0.75f;
    return TextColor;
}

FLinearColor SGraphNode_GridFlowAbstractNode::GetTextShadowColor() const {
    return FLinearColor::Transparent;
}

void SGraphNode_GridFlowAbstractNode::CreateNodeItemWidgets() {
    NodeItemWidgets.Reset();

    if (UGridFlowAbstractEdGraphNode* EdNode = Cast<UGridFlowAbstractEdGraphNode>(GetNodeObj())) {
        if (EdNode->ScriptNode.IsValid()) {
            const TArray<UFlowGraphItem*>& Items = EdNode->ScriptNode->NodeItems;
            for (const UFlowGraphItem* Item : Items) {
                TSharedPtr<SGridFlowItemOverlay> ItemWidget =
                    SNew(SGridFlowItemOverlay, Item)
                    .Selected(this, &SGraphNode_GridFlowAbstractNode::IsItemSelected, Item->ItemId);
                ItemWidget->GetOnMousePressed().BindRaw(this, &SGraphNode_GridFlowAbstractNode::OnItemClicked);

                NodeItemWidgets.Add(ItemWidget);
            }
        }
    }
}

void SGraphNode_GridFlowAbstractNode::CreateLinkItemWidgets() {
    LinkItemWidgets.Reset();
    if (UGridFlowAbstractEdGraphNode* EdNode = Cast<UGridFlowAbstractEdGraphNode>(GetNodeObj())) {
        UGridFlowAbstractEdGraph* EdGraph = Cast<UGridFlowAbstractEdGraph>(EdNode->GetGraph());
        if (EdGraph->ScriptGraph.IsValid()) {
            UGridFlowAbstractGraph* ScriptGraph = EdGraph->ScriptGraph.Get();
            for (const UFlowAbstractLink* Link : ScriptGraph->GraphLinks) {
                if (Link->Source == EdNode->NodeGuid) {
                    // This is an outgoing link. Create items, if any
                    for (const UFlowGraphItem* Item : Link->LinkItems) {
                        TSharedPtr<SGridFlowItemOverlay> ItemWidget = SNew(SGridFlowItemOverlay, Item)
                            .Selected(this, &SGraphNode_GridFlowAbstractNode::IsItemSelected, Item->ItemId);
                        ItemWidget->GetOnMousePressed().BindRaw(this, &SGraphNode_GridFlowAbstractNode::OnItemClicked);

                        FLinkItemWidgetInfo LinkInfo;
                        LinkInfo.ItemWidget = ItemWidget;
                        LinkInfo.DestinationNodeId = Link->Destination;
                        LinkItemWidgets.Add(LinkInfo);
                    }
                }
            }
        }
    }
}

void SGraphNode_GridFlowAbstractNode::OnItemClicked(const FGuid& InItemId, bool bDoubleClicked) {
    UGridFlowAbstractEdGraph* Graph = Cast<UGridFlowAbstractEdGraph>(GetNodeObj()->GetGraph());
    if (Graph) {
        Graph->OnItemWidgetClicked.ExecuteIfBound(InItemId, bDoubleClicked);
    }
}

bool SGraphNode_GridFlowAbstractNode::IsItemSelected(FGuid InItemId) const {
    if (UGridFlowAbstractEdGraph* Graph = Cast<UGridFlowAbstractEdGraph>(GetNodeObj()->GetGraph())) {
        return Graph->SelectedItemId == InItemId;
    }
    return false;
}

TArray<FOverlayWidgetInfo> SGraphNode_GridFlowAbstractNode::GetOverlayWidgets(
    bool bSelected, const FVector2D& WidgetSize) const {
    TArray<FOverlayWidgetInfo> Overlays;

    float HostRadius = WidgetSize.X * 0.5f;
    if (HostRadius == 0) {
        HostRadius = 32;
    }
    const FVector2D Origin(HostRadius, HostRadius);

    // Add the node item widgets
    {
        float AngleIncrement = 0;
        if (NodeItemWidgets.Num() > 0) {
            AngleIncrement = 360.0f / NodeItemWidgets.Num();
        }

        float Angle = -90;
        for (TSharedPtr<SGridFlowItemOverlay> ItemWidget : NodeItemWidgets) {
            const float ItemRadius = ItemWidget->GetWidgetRadius();
            const float OffsetDistance = HostRadius - ItemRadius;
            FVector2D BaseOffset = FVector2D(
                    FMath::Cos(FMath::DegreesToRadians(Angle)),
                    FMath::Sin(FMath::DegreesToRadians(Angle)))
                * OffsetDistance;
            ItemWidget->SetBaseOffset(BaseOffset);

            FVector2D Offset = BaseOffset - FVector2D(ItemRadius, ItemRadius);
            Angle += AngleIncrement;

            FOverlayWidgetInfo Overlay(ItemWidget);
            Overlay.OverlayOffset = Origin + Offset;
            Overlays.Add(Overlay);
        }
    }

    // Add the link item widgets 
    if (LinkItemWidgets.Num() > 0) {
        UEdGraphNode* SourceNode = GetNodeObj();
        TMap<FGuid, UEdGraphNode*> NodeMap;
        for (UEdGraphNode* Node : SourceNode->GetGraph()->Nodes) {
            NodeMap.Add(Node->NodeGuid, Node);
        }
        static const FVector2D NodeCenterOffset(32, 32);

        for (const FLinkItemWidgetInfo& LinkItemInfo : LinkItemWidgets) {
            // Update the base offset
            UEdGraphNode** DestNodePtr = NodeMap.Find(LinkItemInfo.DestinationNodeId);
            if (DestNodePtr) {
                UEdGraphNode* DestNode = *DestNodePtr;

                FVector2D SrcLocation = FVector2D(SourceNode->NodePosX, SourceNode->NodePosY);
                FVector2D DstLocation = FVector2D(DestNode->NodePosX, DestNode->NodePosY);

                FVector2D BaseOffset = (DstLocation - SrcLocation) * 0.5f;
                LinkItemInfo.ItemWidget->SetBaseOffset(BaseOffset);

                const float ItemRadius = LinkItemInfo.ItemWidget->GetWidgetRadius();
                FVector2D Offset = BaseOffset - FVector2D(ItemRadius, ItemRadius);
                FOverlayWidgetInfo Overlay(LinkItemInfo.ItemWidget);
                Overlay.OverlayOffset = Origin + Offset;
                Overlays.Add(Overlay);
            }
        }
    }

    return Overlays;
}

FVector2D SGraphNode_GridFlowAbstractNode::ComputeDesiredSize(float LayoutScaleMultiplier) const {
    FVector2D Size = SGraphNode::ComputeDesiredSize(LayoutScaleMultiplier);
    return Size;
}

