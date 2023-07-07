//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/ExecutionGraph/Widgets/SGraphNode_ExecRuleNode.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecRuleNode.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Widgets/SGraphPin_ExecNodePin.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SGraphNode_ExecRuleNode"


/////////////////////////////////////////////////////
// SExecNodeOutputPin

class SExecNodeOutputPin : public SGraphPin {
public:
    SLATE_BEGIN_ARGS(SExecNodeOutputPin) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
    // Begin SGraphPin interface
    virtual TSharedRef<SWidget> GetDefaultValueWidget() override;
    // End SGraphPin interface

    const FSlateBrush* GetPinBorder() const;
};

void SExecNodeOutputPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin) {
    this->SetCursor(EMouseCursor::Default);

    typedef SExecNodeOutputPin ThisClass;

    bShowLabel = true;

    GraphPinObj = InPin;
    check(GraphPinObj != NULL);

    const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
    check(Schema);

    // Set up a hover for pins that is tinted the color of the pin.
    SBorder::Construct(SBorder::FArguments()
                       .BorderImage(this, &SExecNodeOutputPin::GetPinBorder)
                       .BorderBackgroundColor(this, &ThisClass::GetPinColor)
                       .OnMouseButtonDown(this, &ThisClass::OnPinMouseDown)
                       .Cursor(this, &ThisClass::GetPinCursor)
    );
}

TSharedRef<SWidget> SExecNodeOutputPin::GetDefaultValueWidget() {
    return SNew(STextBlock);
}

const FSlateBrush* SExecNodeOutputPin::GetPinBorder() const {
    return (IsHovered())
               ? FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.BackgroundHovered"))
               : FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.Background"));
}

/////////////////////////////////////////////////////
// SGraphNode_ExecRuleNode

void SGraphNode_ExecRuleNode::Construct(const FArguments& InArgs, UEdGraphNode_ExecNodeBase* InNode) {
    GraphNode = InNode;

    this->SetCursor(EMouseCursor::CardinalCross);

    this->UpdateGraphNode();
}

FLinearColor SGraphNode_ExecRuleNode::InactiveStateColor(0.08f, 0.08f, 0.08f);
FLinearColor SGraphNode_ExecRuleNode::ActiveStateColorDim(0.4f, 0.3f, 0.15f);
FLinearColor SGraphNode_ExecRuleNode::ActiveStateColorBright(1.f, 0.6f, 0.35f);
FLinearColor SGraphNode_ExecRuleNode::ErrorColor(1.f, 0.1f, 0.0f);

FSlateColor SGraphNode_ExecRuleNode::GetBorderBackgroundColor() const {
    UEdGraphNode_ExecRuleNode* RuleNode = Cast<UEdGraphNode_ExecRuleNode>(GetNodeObj());
    if (RuleNode && !RuleNode->Rule.IsValid()) {
        return ErrorColor;
    }
    return InactiveStateColor;
}

const FSlateBrush* SGraphNode_ExecRuleNode::GetNameIcon() const {
    return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}

FText SGraphNode_ExecRuleNode::GetExecutionTypeText() const {
    UEdGraphNode_ExecRuleNode* RuleNode = Cast<UEdGraphNode_ExecRuleNode>(GetNodeObj());
    if (RuleNode) {
        if (RuleNode->ExecutionMode == ERuleNodeExecutionMode::RunOnce) {
            return LOCTEXT("NodeLable", "Run Once");
        }
        if (RuleNode->ExecutionMode == ERuleNodeExecutionMode::RunWithProbability) {
            FFormatNamedArguments Args;
            float Percent = RuleNode->ExecutionConfig.RunProbability * 100;
            Percent = FMath::Clamp(Percent, 0.0f, 100.0f);
            Args.Add(TEXT("Percent"), Percent);
            return FText::Format(LOCTEXT("RuleNodeExecPattern_RunWithProbability",
                                         "Run Probability: {Percent}%"), Args);
        }
        if (RuleNode->ExecutionMode == ERuleNodeExecutionMode::Iterate) {
            FFormatNamedArguments Args;
            Args.Add(TEXT("Count"), RuleNode->ExecutionConfig.IterationCount);
            return FText::Format(LOCTEXT("RuleNodeExecPattern_Iterate", "Run {Count} times"), Args);
        }
        if (RuleNode->ExecutionMode == ERuleNodeExecutionMode::IterateRange) {
            int Start = RuleNode->ExecutionConfig.IterationCountMin;
            int End = FMath::Max(Start, RuleNode->ExecutionConfig.IterationCountMax);
            if (Start == End) {
                FFormatNamedArguments Args;
                Args.Add(TEXT("Count"), Start);
                return FText::Format(LOCTEXT("RuleNodeExecPattern_IterateRange1", "Run {Count} times"), Args);
            }
            FFormatNamedArguments Args;
            Args.Add(TEXT("Start"), Start);
            Args.Add(TEXT("End"), End);
            return FText::Format(LOCTEXT("RuleNodeExecPattern_IterateRange2", "Run {Start}-{End} times"), Args);
        }
    }

    return FText::GetEmpty();
}

void SGraphNode_ExecRuleNode::UpdateGraphNode() {
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
        SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor(this, &SGraphNode_ExecRuleNode::GetBorderBackgroundColor)
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
								.Text(this, &SGraphNode_ExecRuleNode::GetEditableNodeTitleAsText)
								.TextStyle(FEditorStyle::Get(), "Graph.StateNode.NodeTitle")

                    ]
                    + SVerticalBox::Slot()
                    [
                        SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.Text(this, &SGraphNode_ExecRuleNode::GetExecutionTypeText)
                    ]
                ]
            ]
        ]
    ];

    CreatePinWidgets();
}

void SGraphNode_ExecRuleNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd) {
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

void SGraphNode_ExecRuleNode::CreatePinWidgets() {
    UEdGraphNode_ExecNodeBase* ExecNode = CastChecked<UEdGraphNode_ExecNodeBase>(GraphNode);

    UEdGraphPin* CurPin = ExecNode->GetOutputPin();
    TSharedPtr<SGraphPin> NewPin = SNew(SExecNodeOutputPin, CurPin);

    this->AddPin(NewPin.ToSharedRef());
}


#undef LOCTEXT_NAMESPACE

