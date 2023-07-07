//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/RuleGraph/Debugging/SGraphNode_DebugGrammarNode.h"

#include "Builders/SnapMap/SnapMapDungeonConfig.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraphNode_DebugGrammarNode.h"
#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraph_DebugGrammar.h"

#include "ConnectionDrawingPolicy.h"
#include "SCommentBubble.h"
#include "SGraphPin.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SGraphNode_DebugGrammarNode"


/////////////////////////////////////////////////////
// SDebugGrammarNodeOutputPin

class SDebugGrammarNodeOutputPin : public SGraphPin {
public:
    SLATE_BEGIN_ARGS(SDebugGrammarNodeOutputPin) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
    // Begin SGraphPin interface
    virtual TSharedRef<SWidget> GetDefaultValueWidget() override;
    // End SGraphPin interface

    const FSlateBrush* GetPinBorder() const;
};

void SDebugGrammarNodeOutputPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin) {
    this->SetCursor(EMouseCursor::Default);

    typedef SDebugGrammarNodeOutputPin ThisClass;

    bShowLabel = true;

    GraphPinObj = InPin;
    check(GraphPinObj != NULL);

    const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
    check(Schema);

    // Set up a hover for pins that is tinted the color of the pin.
    SBorder::Construct(SBorder::FArguments()
                       .BorderImage(this, &SDebugGrammarNodeOutputPin::GetPinBorder)
                       .BorderBackgroundColor(this, &ThisClass::GetPinColor)
                       .OnMouseButtonDown(this, &ThisClass::OnPinMouseDown)
                       .Cursor(this, &ThisClass::GetPinCursor)
    );
}

TSharedRef<SWidget> SDebugGrammarNodeOutputPin::GetDefaultValueWidget() {
    return SNew(STextBlock);
}

const FSlateBrush* SDebugGrammarNodeOutputPin::GetPinBorder() const {
    return (IsHovered())
               ? FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.BackgroundHovered"))
               : FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.Background"));
}

/////////////////////////////////////////////////////
// SDebugGrammarNodeDoorWidget

class DUNGEONARCHITECTEDITOR_API SDebugGrammarNodeDoorWidget : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SDebugGrammarNodeDoorWidget) {}
        SLATE_ARGUMENT(bool, IsIncomingDoor)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs) {
        bool bIsIncomingDoor = InArgs._IsIncomingDoor;
        const FSlateBrush* DoorIcon = bIsIncomingDoor
                                          ? FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.DebugDoorIconIn"))
                                          : FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.DebugDoorIconOut"));

        this->ChildSlot
        [
            SNew(SBorder)
            .Padding(2)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                  .AutoHeight()
                  .VAlign(VAlign_Center)
                [
                    SNew(SImage)
                    .Image(DoorIcon)
                ]
            ]
        ];

    }
};


/////////////////////////////////////////////////////
// SGraphNode_DebugGrammarNode

void SGraphNode_DebugGrammarNode::Construct(const FArguments& InArgs, UEdGraphNode_DebugGrammarNode* InNode) {
    GraphNode = InNode;

    this->SetCursor(EMouseCursor::CardinalCross);

    this->UpdateGraphNode();
}

FLinearColor SGraphNode_DebugGrammarNode::InactiveStateColor(0.08f, 0.08f, 0.08f);
FLinearColor SGraphNode_DebugGrammarNode::ActiveStateColorDim(0.4f, 0.3f, 0.15f);
FLinearColor SGraphNode_DebugGrammarNode::ActiveStateColorBright(1.f, 0.6f, 0.35f);
FLinearColor SGraphNode_DebugGrammarNode::ErrorColor(1.f, 0.1f, 0.0f);

FLinearColor SGraphNode_DebugGrammarNode::CurrentNodeColor(0.2f, 0.2f, 1.0f);
FLinearColor SGraphNode_DebugGrammarNode::ProcessedNodeColor(0.1f, 0.6f, 0.1f);

FSlateColor SGraphNode_DebugGrammarNode::GetBorderBackgroundColor() const {
    UEdGraphNode_DebugGrammarNode* GrammarNode = Cast<UEdGraphNode_DebugGrammarNode>(GetNodeObj());
    if (GrammarNode && !GrammarNode->TypeInfo.IsValid()) {
        return ErrorColor;
    }

    if (GrammarNode) {
        UEdGraph_DebugGrammar* DebugGraph = Cast<UEdGraph_DebugGrammar>(GrammarNode->GetGraph());
        bool bCurrentNode = DebugGraph->GetCurrentNode() == GrammarNode;
        if (bCurrentNode) {
            return CurrentNodeColor;
        }

        if (GrammarNode->bProcessed) {
            return ProcessedNodeColor;
        }
    }
    return InactiveStateColor;
}

const FSlateBrush* SGraphNode_DebugGrammarNode::GetNameIcon() const {
    return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}

FText SGraphNode_DebugGrammarNode::GetText_AssignedMap() const {
    UEdGraphNode_DebugGrammarNode* GrammarNode = Cast<UEdGraphNode_DebugGrammarNode>(GetNodeObj());
    if (GrammarNode) {
        UEdGraph_DebugGrammar* DebugGraph = Cast<UEdGraph_DebugGrammar>(GrammarNode->GetGraph());
        const bool bIsSelectedNode = (DebugGraph->GetCurrentNode() == GrammarNode);
        if (GrammarNode->bModuleAssigned || bIsSelectedNode) {
            const FString LevelName = GrammarNode->ModuleLevel.GetAssetName();
            return FText::FromString(LevelName);
        }
    }
    return LOCTEXT("AssignedMapText", "Level: None");
}

void SGraphNode_DebugGrammarNode::UpdateGraphNode() {
    InputPins.Empty();
    OutputPins.Empty();

    // Reset variables that are going to be exposed, in case we are refreshing an already setup node.
    RightNodeBox.Reset();
    LeftNodeBox.Reset();

    const FSlateBrush* NodeTypeIcon = GetNameIcon();

    FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
    TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

    this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
    this->GetOrAddSlot(ENodeZone::Center)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [
            SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
				.Padding(0)
				.BorderBackgroundColor(this, &SGraphNode_DebugGrammarNode::GetBorderBackgroundColor)
            [
                SNew(SOverlay)
                // PIN AREA
                + SOverlay::Slot()
                  .HAlign(HAlign_Fill)
                  .VAlign(VAlign_Fill)
                [
                    SAssignNew(RightNodeBox, SVerticalBox)
                ]

                // STATE NAME AREA
                + SOverlay::Slot()
                  .HAlign(HAlign_Center)
                  .VAlign(VAlign_Center)
                  .Padding(10.0f)
                [
                    SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.ColorSpill"))
						.BorderBackgroundColor(TitleShadowColor)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Visibility(EVisibility::SelfHitTestInvisible)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
                        [
                            SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.Text(this, &SGraphNode_DebugGrammarNode::GetEditableNodeTitleAsText)
								.TextStyle(FEditorStyle::Get(), "Graph.StateNode.NodeTitle")
                        ]

                        + SVerticalBox::Slot()
                        [
                            SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.Text(this, &SGraphNode_DebugGrammarNode::GetText_AssignedMap)
                        ]
                    ]
                ]
            ]
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SBorder)
				.Visibility(this, &SGraphNode_DebugGrammarNode::GetStatusMessageVisibility)
				.BorderBackgroundColor(this, &SGraphNode_DebugGrammarNode::GetStatusMessageColor)
            [
                SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
					.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
					.Text(this, &SGraphNode_DebugGrammarNode::GetStatusMessageText)
            ]
        ]
    ];

    CreatePinWidgets();
}

void SGraphNode_DebugGrammarNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd) {
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

void SGraphNode_DebugGrammarNode::CreatePinWidgets() {
    UEdGraphNode_GrammarNode* TaskNode = CastChecked<UEdGraphNode_GrammarNode>(GraphNode);

    UEdGraphPin* CurPin = TaskNode->GetOutputPin();
    TSharedPtr<SGraphPin> NewPin = SNew(SDebugGrammarNodeOutputPin, CurPin);

    this->AddPin(NewPin.ToSharedRef());
}

FSlateColor SGraphNode_DebugGrammarNode::GetStatusMessageColor() const {
    return FLinearColor(0.2, 0.2, 1.0);
}

EVisibility SGraphNode_DebugGrammarNode::GetStatusMessageVisibility() const {
    UEdGraph_DebugGrammar* DebugGraph = Cast<UEdGraph_DebugGrammar>(GetNodeObj()->GetGraph());
    bool bCurrentNode = (DebugGraph->GetCurrentNode() == GetNodeObj());
    return bCurrentNode ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

FText SGraphNode_DebugGrammarNode::GetStatusMessageText() const {
    UEdGraphNode_DebugGrammarNode* DebugNode = CastChecked<UEdGraphNode_DebugGrammarNode>(GetNodeObj());
    return FText::FromString(DebugNode->StatusMessage);
}

////////////////////////////////// SGraphNode_DebugGrammarDoorNode ////////////////////////////////// 

void SGraphNode_DebugGrammarDoorNode::Construct(const FArguments& InArgs, UEdGraphNode_DebugGrammarDoorNode* InNode) {
    GraphNode = InNode;
    this->SetCursor(EMouseCursor::CardinalCross);
    this->UpdateGraphNode();
}

void SGraphNode_DebugGrammarDoorNode::UpdateGraphNode() {
    InputPins.Empty();
    OutputPins.Empty();

    // Reset variables that are going to be exposed, in case we are refreshing an already setup node.
    RightNodeBox.Reset();
    LeftNodeBox.Reset();

    const FSlateBrush* DoorIcon = FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.DebugDoorIcon"));

    FLinearColor BackgroundColor(0.3f, 0.3f, 0.3f);
    this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

    this->GetOrAddSlot(ENodeZone::Center)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
    [
        SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.ColorSpill"))
			.ForegroundColor(FSlateColor(BackgroundColor))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Visibility(this, &SGraphNode_DebugGrammarDoorNode::IsNodeVisible)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
              .AutoHeight()
              .HAlign(HAlign_Center)
              .VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                  .AutoHeight()
                  .VAlign(VAlign_Center)
                [
                    SNew(SImage)
                    .Image(DoorIcon)
                ]
            ]
            + SVerticalBox::Slot()
            [
                SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text(this, &SGraphNode_DebugGrammarDoorNode::GetDoorDataText)
            ]
        ]
    ];
}

void SGraphNode_DebugGrammarDoorNode::GetNodeInfoPopups(FNodeInfoContext* Context,
                                                        TArray<FGraphInformationPopupInfo>& Popups) const {

}

void SGraphNode_DebugGrammarDoorNode::MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter) {
    // Ignored; position is set by the location of the attached state nodes
}

bool SGraphNode_DebugGrammarDoorNode::RequiresSecondPassLayout() const {
    return true;
}

void SGraphNode_DebugGrammarDoorNode::PerformSecondPassLayout(
    const TMap<UObject*, TSharedRef<SNode>>& NodeToWidgetLookup) const {
    FGeometry StartGeom;
    FGeometry EndGeom;
    FGeometry DoorGeom;

    UEdGraphNode_DebugGrammarDoorNode* DoorNode = CastChecked<UEdGraphNode_DebugGrammarDoorNode>(GraphNode);
    UEdGraphNode_DebugGrammarNode* NodeStart = DoorNode->Incoming;
    UEdGraphNode_DebugGrammarNode* NodeEnd = DoorNode->Outgoing;

    if (NodeStart && NodeEnd) {
        const TSharedRef<SNode>* NodeStartWidgetPtr = NodeToWidgetLookup.Find(NodeStart);
        const TSharedRef<SNode>* NodeEndWidgetPtr = NodeToWidgetLookup.Find(NodeEnd);
        if (NodeStartWidgetPtr && NodeEndWidgetPtr) {
            const TSharedRef<SNode> NodeStartWidget = *NodeStartWidgetPtr;
            const TSharedRef<SNode> NodeEndWidget = *NodeEndWidgetPtr;

            StartGeom = FGeometry(FVector2D(NodeStart->NodePosX, NodeStart->NodePosY), FVector2D::ZeroVector,
                                  NodeStartWidget->GetDesiredSize(), 1.0f);
            EndGeom = FGeometry(FVector2D(NodeEnd->NodePosX, NodeEnd->NodePosY), FVector2D::ZeroVector,
                                NodeEndWidget->GetDesiredSize(), 1.0f);
        }
    }

    PositionBetweenTwoNodes(StartGeom, EndGeom);
}

void SGraphNode_DebugGrammarDoorNode::PositionBetweenTwoNodes(const FGeometry& StartGeom,
                                                              const FGeometry& EndGeom) const {
    // Get a reasonable seed point (halfway between the boxes)
    const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
    const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
    const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

    // Find the (approximate) closest points between the two boxes
    const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
    const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

    const FVector2D TargetCenter = (StartAnchorPoint + EndAnchorPoint) / 2.0f;
    const FVector2D DesiredNodeSize = GetDesiredSize();
    const FVector2D TargetLocation = TargetCenter - DesiredNodeSize * 0.5f;

    GraphNode->NodePosX = TargetLocation.X;
    GraphNode->NodePosY = TargetLocation.Y;
}

FText SGraphNode_DebugGrammarDoorNode::GetDoorDataText() const {
    FString Label = "";
    if (UEdGraphNode_DebugGrammarDoorNode* DoorNode = CastChecked<UEdGraphNode_DebugGrammarDoorNode>(GraphNode)) {
        if (DoorNode->Outgoing) {
            Label = FString::Printf(TEXT("%d - %d"), DoorNode->Outgoing->RemoteDoorIndex,
                                    DoorNode->Outgoing->IncomingDoorIndex);
        }
    }
    return FText::FromString(Label);
}

EVisibility SGraphNode_DebugGrammarDoorNode::IsNodeVisible() const {
    UEdGraphNode_DebugGrammarDoorNode* DoorNode = CastChecked<UEdGraphNode_DebugGrammarDoorNode>(GraphNode);
    if (DoorNode && DoorNode->Incoming && DoorNode->Outgoing) {
        if (DoorNode->Incoming->bProcessed && DoorNode->Outgoing->bProcessed) {
            return EVisibility::HitTestInvisible;
        }
    }

    return EVisibility::Collapsed;
}


#undef LOCTEXT_NAMESPACE

