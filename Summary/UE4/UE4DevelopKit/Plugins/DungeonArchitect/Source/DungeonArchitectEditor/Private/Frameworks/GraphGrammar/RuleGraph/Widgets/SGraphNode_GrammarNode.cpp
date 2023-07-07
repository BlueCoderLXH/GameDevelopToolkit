//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/RuleGraph/Widgets/SGraphNode_GrammarNode.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"
#include "Frameworks/GraphGrammar/RuleGraph/Widgets/SGraphPin_GrammarNodePin.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SGraphNode_GrammarNode"


/////////////////////////////////////////////////////
// SGrammarNodeOutputPin

class SGrammarNodeOutputPin : public SGraphPin {
public:
    SLATE_BEGIN_ARGS(SGrammarNodeOutputPin) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
    // Begin SGraphPin interface
    virtual TSharedRef<SWidget> GetDefaultValueWidget() override;
    // End SGraphPin interface

    const FSlateBrush* GetPinBorder() const;

};

void SGrammarNodeOutputPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin) {
    this->SetCursor(EMouseCursor::Default);

    typedef SGrammarNodeOutputPin ThisClass;

    bShowLabel = true;

    GraphPinObj = InPin;
    check(GraphPinObj != NULL);

    const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
    check(Schema);

    // Set up a hover for pins that is tinted the color of the pin.
    SBorder::Construct(SBorder::FArguments()
                       .BorderImage(this, &SGrammarNodeOutputPin::GetPinBorder)
                       .BorderBackgroundColor(this, &ThisClass::GetPinColor)
                       .OnMouseButtonDown(this, &ThisClass::OnPinMouseDown)
                       .Cursor(this, &ThisClass::GetPinCursor)
    );
}

TSharedRef<SWidget> SGrammarNodeOutputPin::GetDefaultValueWidget() {
    return SNew(STextBlock);
}


const FSlateBrush* SGrammarNodeOutputPin::GetPinBorder() const {
    return (IsHovered())
               ? FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.BackgroundHovered"))
               : FDungeonArchitectStyle::Get().GetBrush(TEXT("DA.SnapEd.StateNode.Pin.Background"));
}

/////////////////////////////////////////////////////
// SGraphNode_GrammarNode

void SGraphNode_GrammarNode::Construct(const FArguments& InArgs, UEdGraphNode_GrammarNode* InNode) {
    GraphNode = InNode;

    this->SetCursor(EMouseCursor::CardinalCross);

    this->UpdateGraphNode();
}

FLinearColor SGraphNode_GrammarNode::InactiveStateColor(0.08f, 0.08f, 0.08f);
FLinearColor SGraphNode_GrammarNode::ActiveStateColorDim(0.4f, 0.3f, 0.15f);
FLinearColor SGraphNode_GrammarNode::ActiveStateColorBright(1.f, 0.6f, 0.35f);
FLinearColor SGraphNode_GrammarNode::ErrorColor(1.f, 0.1f, 0.0f);

FSlateColor SGraphNode_GrammarNode::GetBorderBackgroundColor() const {
    UEdGraphNode_GrammarNode* GrammarNode = Cast<UEdGraphNode_GrammarNode>(GetNodeObj());
    if (GrammarNode) {
        return GrammarNode->TypeInfo.IsValid()
            ? GrammarNode->TypeInfo->NodeColor
            : ErrorColor;
    }
    
    return InactiveStateColor;
}

const FSlateBrush* SGraphNode_GrammarNode::GetNameIcon() const {
    return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}

void SGraphNode_GrammarNode::UpdateGraphNode() {
    InputPins.Empty();
    OutputPins.Empty();

    // Reset variables that are going to be exposed, in case we are refreshing an already setup node.
    RightNodeBox.Reset();
    LeftNodeBox.Reset();

    const FSlateBrush* NodeTypeIcon = GetNameIcon();

    FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
    TSharedPtr<SErrorText> ErrorText;
    TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

    this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
    this->GetOrAddSlot(ENodeZone::Center)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
    [
        SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor(this, &SGraphNode_GrammarNode::GetBorderBackgroundColor)
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
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        // POPUP ERROR MESSAGE
                        SAssignNew(ErrorText, SErrorText)
                        //.BackgroundColor(this, &SGraphNodeAnimState::GetErrorColor)
                        //.ToolTipText(this, &SGraphNodeAnimState::GetErrorMsgToolTip)
                    ]
                    /*
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        [
                            SNew(SImage)
                            .Image(NodeTypeIcon)
                        ]
                    */
                    + SHorizontalBox::Slot()
                    .Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
                    [
                        SNew(STextBlock)
									.Text(this, &SGraphNode_GrammarNode::GetEditableNodeTitleAsText)
									.TextStyle(FEditorStyle::Get(), "Graph.StateNode.NodeTitle")

                    ]
                ]
            ]
        ]
    ];

    ErrorReporting = ErrorText;
    ErrorReporting->SetError(ErrorMsg);
    CreatePinWidgets();
}

void SGraphNode_GrammarNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd) {
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

void SGraphNode_GrammarNode::CreatePinWidgets() {
    UEdGraphNode_GrammarNode* TaskNode = CastChecked<UEdGraphNode_GrammarNode>(GraphNode);

    UEdGraphPin* CurPin = TaskNode->GetOutputPin();
    TSharedPtr<SGraphPin> NewPin = SNew(SGrammarNodeOutputPin, CurPin);

    this->AddPin(NewPin.ToSharedRef());
}


#undef LOCTEXT_NAMESPACE

