//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/SGraphNode_DungeonMarkerEmitter.h"

#include "Core/Editors/ThemeEditor/Widgets/SDungeonOutputPin.h"

#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SGraphNode_DungeonMarkerEmitter"


#undef LOCTEXT_NAMESPACE

void SGraphNode_DungeonMarkerEmitter::Construct(const FArguments& InArgs, UEdGraphNode_DungeonMarkerEmitter* InNode) {
    GraphNode = InNode;

    this->SetCursor(EMouseCursor::CardinalCross);

    this->UpdateGraphNode();
}

FLinearColor SGraphNode_DungeonMarkerEmitter::InactiveStateColor(0.08f, 0.08f, 0.25f);
FLinearColor SGraphNode_DungeonMarkerEmitter::ActiveStateColorDim(0.4f, 0.3f, 0.15f);
FLinearColor SGraphNode_DungeonMarkerEmitter::ActiveStateColorBright(1.f, 0.6f, 0.35f);

FSlateColor SGraphNode_DungeonMarkerEmitter::GetBorderBackgroundColor() const {
    return InactiveStateColor;
}

const FSlateBrush* SGraphNode_DungeonMarkerEmitter::GetNameIcon() const {
    return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}


bool SGraphNode_DungeonMarkerEmitter::IsNameReadOnly() const {
    return true;
}

void SGraphNode_DungeonMarkerEmitter::UpdateGraphNode() {
    InputPins.Empty();
    OutputPins.Empty();

    // Reset variables that are going to be exposed, in case we are refreshing an already setup node.
    RightNodeBox.Reset();
    LeftNodeBox.Reset();
    OutputPinBox.Reset();

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
			.BorderBackgroundColor(this, &SGraphNode_DungeonMarkerEmitter::GetBorderBackgroundColor)
        [
            SNew(SOverlay)


            // INPUT PIN AREA
            + SOverlay::Slot()
              .HAlign(HAlign_Fill)
              .VAlign(VAlign_Top)
            [
                SAssignNew(LeftNodeBox, SVerticalBox)
            ]

            // OUTPUT PIN AREA
            + SOverlay::Slot()
              .HAlign(HAlign_Fill)
              .VAlign(VAlign_Bottom)
            [
                SAssignNew(RightNodeBox, SVerticalBox)
                + SVerticalBox::Slot()
                  .HAlign(HAlign_Fill)
                  .VAlign(VAlign_Fill)
                  .Padding(20.0f, 0.0f)
                  .FillHeight(1.0f)
                [
                    SAssignNew(OutputPinBox, SHorizontalBox)
                ]
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
                    + SHorizontalBox::Slot()
                      .AutoWidth()
                      .VAlign(VAlign_Center)
                    [
                        SNew(SImage)
                        .Image(NodeTypeIcon)
                    ]
                    + SHorizontalBox::Slot()
                    .Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SAssignNew(InlineEditableText, SInlineEditableTextBlock)
										.Style(FEditorStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
										.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
										.IsReadOnly(this, &SGraphNode_DungeonMarkerEmitter::IsNameReadOnly)
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            NodeTitle.ToSharedRef()
                        ]
                    ]
                ]
            ]
        ]
    ];

    ErrorReporting = ErrorText;
    ErrorReporting->SetError(ErrorMsg);
    CreatePinWidgets();
}

void SGraphNode_DungeonMarkerEmitter::CreatePinWidgets() {
    UEdGraphNode_DungeonMarkerEmitter* DungeonNode = CastChecked<UEdGraphNode_DungeonMarkerEmitter>(GraphNode);

    //{
    //	UEdGraphPin* CurPin = DungeonNode->GetOutputPin();
    //	TSharedPtr<SGraphPin> NewPin = SNew(SDungeonOutputPin, CurPin);
    //	NewPin->SetIsEditable(IsEditable);
    //	this->AddPin(NewPin.ToSharedRef());
    //	OutputPins.Add(NewPin.ToSharedRef());
    //}
    {
        UEdGraphPin* CurPin = DungeonNode->GetInputPin();
        TSharedPtr<SGraphPin> NewPin = SNew(SDungeonOutputPin, CurPin);
        NewPin->SetIsEditable(IsEditable);
        this->AddPin(NewPin.ToSharedRef());
        InputPins.Add(NewPin.ToSharedRef());
    }
}

void SGraphNode_DungeonMarkerEmitter::AddPin(const TSharedRef<SGraphPin>& PinToAdd) {
    PinToAdd->SetOwner(SharedThis(this));

    const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
    const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
    if (bAdvancedParameter) {
        PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
    }

    if (PinToAdd->GetDirection() == EGPD_Input) {
        LeftNodeBox->AddSlot()
                   .HAlign(HAlign_Fill)
                   .VAlign(VAlign_Fill)
                   .FillHeight(1.0f)
                   .Padding(20.0f, 0.0f)
        [
            PinToAdd
        ];
        InputPins.Add(PinToAdd);
    }
    else // Direction == EEdGraphPinDirection::EGPD_Output
    {
        OutputPinBox->AddSlot()
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Fill)
                    .FillWidth(1.0f)
        [
            PinToAdd
        ];
        OutputPins.Add(PinToAdd);
    }
}

void SGraphNode_DungeonMarkerEmitter::OnPropertyChanged(UEdGraphNode_DungeonBase* Sender, const FName& PropertyName) {
    UpdateGraphNode();
}

