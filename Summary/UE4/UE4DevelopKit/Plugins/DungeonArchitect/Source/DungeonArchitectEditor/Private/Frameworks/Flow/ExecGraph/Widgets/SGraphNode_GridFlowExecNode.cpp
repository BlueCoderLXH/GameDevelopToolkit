//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/ExecGraph/Widgets/SGraphNode_GridFlowExecNode.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "Frameworks/Flow/ExecGraph/Nodes/GridFlowExecEdGraphNodeBase.h"
#include "Frameworks/Flow/ExecGraph/Nodes/GridFlowExecEdGraphNodes.h"

#include "SGraphPin.h"
#include "Widgets/Notifications/SPopUpErrorText.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SGraphNode_GridFlowExecNode"


/////////////////////////////////////////////////////
// SGridFlowExecNodeOutputPin

class SGridFlowExecNodeOutputPin : public SGraphPin {
public:
    SLATE_BEGIN_ARGS(SGridFlowExecNodeOutputPin) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
    // Begin SGraphPin interface
    virtual TSharedRef<SWidget> GetDefaultValueWidget() override;
    // End SGraphPin interface

    const FSlateBrush* GetPinBorder() const;
};

void SGridFlowExecNodeOutputPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin) {
    this->SetCursor(EMouseCursor::Default);

    typedef SGridFlowExecNodeOutputPin ThisClass;

    bShowLabel = true;

    GraphPinObj = InPin;
    check(GraphPinObj != NULL);

    const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
    check(Schema);

    // Set up a hover for pins that is tinted the color of the pin.
    SBorder::Construct(SBorder::FArguments()
                       .BorderImage(this, &SGridFlowExecNodeOutputPin::GetPinBorder)
                       .BorderBackgroundColor(this, &ThisClass::GetPinColor)
                       .OnMouseButtonDown(this, &ThisClass::OnPinMouseDown)
                       .Cursor(this, &ThisClass::GetPinCursor)
    );
}

TSharedRef<SWidget> SGridFlowExecNodeOutputPin::GetDefaultValueWidget() {
    return SNew(STextBlock);
}

const FSlateBrush* SGridFlowExecNodeOutputPin::GetPinBorder() const {
    return (IsHovered())
               ? FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.BackgroundHovered"))
               : FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.Background"));
}

/////////////////////////////////////////////////////
// SGraphNode_GridFlowExecNode

void SGraphNode_GridFlowExecNode::Construct(const FArguments& InArgs, UGridFlowExecEdGraphNodeBase* InNode) {
    GraphNode = InNode;
    DefaultNodeBorderColor = FLinearColor(0.08f, 0.08f, 0.08f);

    this->SetCursor(EMouseCursor::CardinalCross);
    this->UpdateGraphNode();
}


FSlateColor SGraphNode_GridFlowExecNode::GetBorderBackgroundColor() const {
    if (UGridFlowExecEdGraphNode_Task* TaskNode = Cast<UGridFlowExecEdGraphNode_Task>(GetNodeObj())) {
        UFlowExecTask* Task = TaskNode->TaskTemplate;
        if (Task) {
            return Task->GetNodeColor();
        }
    }
    return DefaultNodeBorderColor;
}

const FSlateBrush* SGraphNode_GridFlowExecNode::GetNameIcon() const {
    return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}

FText SGraphNode_GridFlowExecNode::GetNodeDescriptionText() const {
    if (UGridFlowExecEdGraphNode_Task* TaskNode = Cast<UGridFlowExecEdGraphNode_Task>(GetNodeObj())) {
        UFlowExecTask* Task = TaskNode->TaskTemplate;
        if (Task) {
            return FText::FromString(Task->Description);
        }
    }

    return FText::GetEmpty();
}

EVisibility SGraphNode_GridFlowExecNode::GetNodeDescriptionVisibility() const {
    if (UGridFlowExecEdGraphNode_Task* TaskNode = Cast<UGridFlowExecEdGraphNode_Task>(GetNodeObj())) {
        UFlowExecTask* Task = TaskNode->TaskTemplate;
        if (Task) {
            return Task->Description.Len() > 0
                       ? EVisibility::Visible
                       : EVisibility::Collapsed;
        }
    }

    return EVisibility::Collapsed;
}

FText SGraphNode_GridFlowExecNode::GetNodeErrorText() const {
    if (UGridFlowExecEdGraphNodeBase* ExecNode = Cast<UGridFlowExecEdGraphNodeBase>(GetNodeObj())) {
        if (ExecNode->ExecutionStage == EFlowTaskExecutionStage::Executed) {
            if (ExecNode->ExecutionResult != EFlowTaskExecutionResult::Success) {
                return FText::FromString(ExecNode->ErrorMessage);
            }
        }
    }
    return FText();
}

void SGraphNode_GridFlowExecNode::UpdateGraphNode() {
    InputPins.Empty();
    OutputPins.Empty();

    // Reset variables that are going to be exposed, in case we are refreshing an already setup node.
    RightNodeBox.Reset();
    LeftNodeBox.Reset();

    const FSlateBrush* NodeTypeIcon = GetNameIcon();

    FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
    TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

    const FSlateBrush* ExecStateImageBrush = FDungeonArchitectStyle::Get().GetBrush("FlowEditor.Node.ExecStatus");
    this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
    this->GetOrAddSlot(ENodeZone::Center)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
    [
        SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor(this, &SGraphNode_GridFlowExecNode::GetBorderBackgroundColor)
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
					.BorderImage(this, &SGraphNode_GridFlowExecNode::GetBorderImage)
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
							.Text(this, &SGraphNode_GridFlowExecNode::GetEditableNodeTitleAsText)
							.TextStyle(FEditorStyle::Get(), "Graph.StateNode.NodeTitle")

                    ]
                    + SVerticalBox::Slot()
                    [
                        SNew(SBorder)
							.Padding(FMargin(2.0f, 4.0f, 2.0f, 0.0f))
							.BorderBackgroundColor(FColor::Transparent)
							.Visibility(this, &SGraphNode_GridFlowExecNode::GetNodeDescriptionVisibility)
                        [
                            SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.Text(this, &SGraphNode_GridFlowExecNode::GetNodeDescriptionText)
                        ]
                    ]

                    + SVerticalBox::Slot()
                      .AutoHeight()
                      .Padding(1.0f)
                    [
                        // POPUP ERROR MESSAGE
                        SAssignNew(ErrorText, SErrorText)
                        .BackgroundColor(FColor::Red)
                    ]
                ]
            ]
        ]
    ];

    CreatePinWidgets();

    ErrorReporting = ErrorText;
    ErrorReporting->SetError(GetNodeErrorText());
}

void SGraphNode_GridFlowExecNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd) {
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

void SGraphNode_GridFlowExecNode::SetBorderColor(const FLinearColor& InNodeBorderColor) {
    DefaultNodeBorderColor = InNodeBorderColor;
}

const FSlateBrush* SGraphNode_GridFlowExecNode::GetBorderImage() const {
    UGridFlowExecEdGraphNodeBase* ExecNode = CastChecked<UGridFlowExecEdGraphNodeBase>(GraphNode);
    FName BrushName = "FlowEditor.ExecNode.Body.Default";
    if (ExecNode) {
        if (ExecNode->ExecutionStage == EFlowTaskExecutionStage::WaitingToExecute) {
            BrushName = "FlowEditor.ExecNode.Body.Orange";
        }
        else if (ExecNode->ExecutionStage == EFlowTaskExecutionStage::Executed) {
            BrushName = (ExecNode->ExecutionResult == EFlowTaskExecutionResult::Success)
                            ? "FlowEditor.ExecNode.Body.Green"
                            : "FlowEditor.ExecNode.Body.Red";
        }
    }
    return FDungeonArchitectStyle::Get().GetBrush(BrushName);
}

void SGraphNode_GridFlowExecNode::CreatePinWidgets() {
    UGridFlowExecEdGraphNodeBase* ExecNode = CastChecked<UGridFlowExecEdGraphNodeBase>(GraphNode);

    UEdGraphPin* CurPin = ExecNode->GetOutputPin();
    if (CurPin) {
        TSharedPtr<SGraphPin> NewPin = SNew(SGridFlowExecNodeOutputPin, CurPin);
        this->AddPin(NewPin.ToSharedRef());
    }
}


#undef LOCTEXT_NAMESPACE

