//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/SGraphNode_DungeonMarker.h"

#include "Core/Editors/ThemeEditor/Graph/EdGraph_DungeonProp.h"
#include "Core/Editors/ThemeEditor/Widgets/SDungeonOutputPin.h"

#include "GraphEditAction.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SGraphNode_DungeonMarker"


#undef LOCTEXT_NAMESPACE

void SGraphNode_DungeonMarker::Construct(const FArguments& InArgs, UEdGraphNode_DungeonMarker* InNode) {
    GraphNode = InNode;

    this->SetCursor(EMouseCursor::CardinalCross);

    this->UpdateGraphNode();
}

FLinearColor SGraphNode_DungeonMarker::InactiveStateColor(0.08f, 0.08f, 0.08f);
FLinearColor SGraphNode_DungeonMarker::ActiveStateColorDim(0.4f, 0.3f, 0.15f);
FLinearColor SGraphNode_DungeonMarker::ActiveStateColorBright(1.f, 0.6f, 0.35f);

FSlateColor SGraphNode_DungeonMarker::GetBorderBackgroundColor() const {
    return InactiveStateColor;
}

const FSlateBrush* SGraphNode_DungeonMarker::GetNameIcon() const {
    return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}

bool SGraphNode_DungeonMarker::IsNameDuplicate(const FString& Name) const {
    UEdGraph* Graph = GraphNode->GetGraph();
    TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
    Graph->GetNodesOfClass<UEdGraphNode_DungeonMarker>(MarkerNodes);
    for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
        if (MarkerNode->MarkerName == Name) {
            // Found a marker node with the specified name
            return true;
        }
    }
    return false;
}

bool SGraphNode_DungeonMarker::IsNodeUserDefined() const {
    UEdGraphNode_DungeonMarker* Marker = Cast<UEdGraphNode_DungeonMarker>(GraphNode);
    return Marker ? Marker->bUserDefined : false;
}

bool SGraphNode_DungeonMarker::IsNameReadOnly() const {
    return !IsNodeUserDefined();
}

void SGraphNode_DungeonMarker::OnPropertyChanged(UEdGraphNode_DungeonBase* Sender, const FName& PropertyName) {
    UpdateGraphNode();
}

void SGraphNode_DungeonMarker::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo) {
    UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(GraphNode);
    MarkerNode->MarkerName = InText.ToString();
    GraphNode->GetGraph()->NotifyGraphChanged();

    UEdGraph_DungeonProp* DungeonGraph = Cast<UEdGraph_DungeonProp>(GraphNode->GetGraph());
    if (DungeonGraph) {
        FEdGraphEditAction PropertyAction(GRAPHACTION_Default, DungeonGraph, MarkerNode, true);
        DungeonGraph->NotifyNodePropertyChanged(PropertyAction);
    }

    UpdateGraphNode();
}

bool SGraphNode_DungeonMarker::OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage) {
    OutErrorMessage = FText::FromString(TEXT("Error"));
    if (InText.ToString().Len() == 0) {
        OutErrorMessage = FText::FromString(TEXT("Invalid Name"));
        return false;
    }

    if (IsNameDuplicate(InText.ToString())) {
        OutErrorMessage = FText::FromString(TEXT("Marker name already exists"));
        return false;
    }

    return true;
}

FString SGraphNode_DungeonMarker::GetNodeErrorText() const {
    UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(GraphNode);
    return MarkerNode ? MarkerNode->ErrorMsg : "";
}

void SGraphNode_DungeonMarker::UpdateGraphNode() {
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
			.BorderBackgroundColor(this, &SGraphNode_DungeonMarker::GetBorderBackgroundColor)
        [
            SNew(SOverlay)
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
                    SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                    [
                        SNew(SHorizontalBox)
                        +SHorizontalBox::Slot()
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
										    .OnVerifyTextChanged(this, &SGraphNode_DungeonMarker::OnVerifyNameTextChanged)
										    .OnTextCommitted(this, &SGraphNode_DungeonMarker::OnNameTextCommited)
										    .IsReadOnly(this, &SGraphNode_DungeonMarker::IsNameReadOnly)
                                //.IsSelected(this, &SGraphNodeAnimState::IsSelectedExclusively)
                            ]
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            [
                                NodeTitle.ToSharedRef()
                            ]
                        ]
                    ]
                    +SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(1.0f)
                    [
                        // POPUP ERROR MESSAGE
                        SAssignNew(ErrorText, SErrorText)
                        .BackgroundColor(FColor::Blue)
                    ]
                ]
            ]
        ]
    ];

    ErrorReporting = ErrorText;
    ErrorReporting->SetError(GetNodeErrorText());
    CreatePinWidgets();
}

void SGraphNode_DungeonMarker::CreatePinWidgets() {
    UEdGraphNode_DungeonMarker* DungeonNode = CastChecked<UEdGraphNode_DungeonMarker>(GraphNode);

    {
        UEdGraphPin* CurPin = DungeonNode->GetOutputPin();
        TSharedPtr<SGraphPin> NewPin = SNew(SDungeonOutputPin, CurPin);
        NewPin->SetIsEditable(IsEditable);
        this->AddPin(NewPin.ToSharedRef());
        OutputPins.Add(NewPin.ToSharedRef());
    }
}

void SGraphNode_DungeonMarker::AddPin(const TSharedRef<SGraphPin>& PinToAdd) {
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

