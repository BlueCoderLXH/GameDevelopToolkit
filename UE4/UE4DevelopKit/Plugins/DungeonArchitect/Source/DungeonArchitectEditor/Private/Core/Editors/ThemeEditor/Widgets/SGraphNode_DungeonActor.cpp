//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/SGraphNode_DungeonActor.h"

#include "Core/Editors/ThemeEditor/DungeonArchitectThemeEditor.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonBase.h"
#include "Core/Editors/ThemeEditor/Widgets/SDungeonOutputPin.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetThumbnail.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "SGraphNode_DungeonActor"


/** Widget for overlaying an execution-order index onto a node */
class SMeshGraphNodeIndex : public SCompoundWidget {
public:
    /** Delegate event fired when the hover state of this widget changes */
    DECLARE_DELEGATE_OneParam(FOnHoverStateChanged, bool /* bHovered */);

    /** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
    DECLARE_DELEGATE_RetVal_OneParam(FSlateColor, FOnGetIndexColor, bool /* bHovered */);

    SLATE_BEGIN_ARGS(SMeshGraphNodeIndex) {}
        SLATE_ATTRIBUTE(FText, Text)
        SLATE_EVENT(FOnHoverStateChanged, OnHoverStateChanged)
        SLATE_EVENT(FOnGetIndexColor, OnGetIndexColor)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs) {
        OnHoverStateChangedEvent = InArgs._OnHoverStateChanged;
        OnGetIndexColorEvent = InArgs._OnGetIndexColor;

        const FSlateBrush* IndexBrush = FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Index"));

        ChildSlot
        [
            SNew(SOverlay)
            + SOverlay::Slot()
              .HAlign(HAlign_Fill)
              .VAlign(VAlign_Fill)
            [
                // Add a dummy box here to make sure the widget doesn't get smaller than the brush
                SNew(SBox)
						.WidthOverride(IndexBrush->ImageSize.X)
						.HeightOverride(IndexBrush->ImageSize.Y)
            ]
            + SOverlay::Slot()
              .HAlign(HAlign_Fill)
              .VAlign(VAlign_Fill)
            [
                SNew(SBorder)
							.BorderImage(IndexBrush)
							.BorderBackgroundColor(this, &SMeshGraphNodeIndex::GetColor)
							.Padding(FMargin(4.0f, 0.0f, 4.0f, 1.0f))
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
                [
                    SNew(STextBlock)
								.Text(InArgs._Text)
								.Font(FEditorStyle::GetFontStyle("BTEditor.Graph.BTNode.IndexText"))
                ]
            ]
        ];
    }

    virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override {
        OnHoverStateChangedEvent.ExecuteIfBound(true);
        SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
    }

    virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override {
        OnHoverStateChangedEvent.ExecuteIfBound(false);
        SCompoundWidget::OnMouseLeave(MouseEvent);
    }

    /** Get the color we use to display the rounded border */
    FSlateColor GetColor() const {
        if (OnGetIndexColorEvent.IsBound()) {
            return OnGetIndexColorEvent.Execute(IsHovered());
        }

        return FSlateColor::UseForeground();
    }

private:
    /** Delegate event fired when the hover state of this widget changes */
    FOnHoverStateChanged OnHoverStateChangedEvent;

    /** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
    FOnGetIndexColor OnGetIndexColorEvent;
};


void SGraphNode_DungeonActor::Construct(const FArguments& InArgs, UEdGraphNode_DungeonActorBase* InNode) {
    ThumbnailSize = FIntPoint(128, 128);
    GraphNode = InNode;
    EdActorNode = InNode;
    SetCursor(EMouseCursor::CardinalCross);
    BuildThumbnailWidget();
    UpdateGraphNode();
}


FSlateColor SGraphNode_DungeonActor::GetBorderBackgroundColor() const {
    static const FLinearColor InactiveStateColor(0.08f, 0.08f, 0.08f);
    static const FLinearColor ActiveStateColorDim(0.4f, 0.3f, 0.15f);
    static const FLinearColor ActiveStateColorBright(1.f, 0.6f, 0.35f);

    return InactiveStateColor;
}

const FSlateBrush* SGraphNode_DungeonActor::GetNameIcon() const {
    return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}

void SGraphNode_DungeonActor::OnPropertyChanged(UEdGraphNode_DungeonBase* Sender, const FName& PropertyName) {
    if (UEdGraphNode_DungeonActorBase* ActorNode = Cast<UEdGraphNode_DungeonActorBase>(Sender)) {
        SetThumbnail(ActorNode->GetThumbnailAssetObject());
        GraphNode->GetGraph()->NotifyGraphChanged();
    }
}

TOptional<float> SGraphNode_DungeonActor::OnGetAffinityValue() const {
    if (UEdGraphNode_DungeonActorBase* ActorNode = Cast<UEdGraphNode_DungeonActorBase>(GraphNode)) {
        return ActorNode->Probability;
    }
    return TOptional<float>();
}

void SGraphNode_DungeonActor::OnAffinityValueChanged(float NewValue) {
    if (UEdGraphNode_DungeonActorBase* ActorNode = Cast<UEdGraphNode_DungeonActorBase>(GraphNode)) {
        ActorNode->Probability = NewValue;
    }
}

void SGraphNode_DungeonActor::OnAffinityValueCommitted(float NewValue, ETextCommit::Type CommitInfo) {
    OnAffinityValueChanged(NewValue);
}

void SGraphNode_DungeonActor::SetThumbnail(UObject* AssetObject) {
    AssetThumbnail->SetAsset(FAssetData(AssetObject));
}

void SGraphNode_DungeonActor::BuildThumbnailWidget() {
    FObjectOrAssetData Value;
    GetValue(Value);
    AssetThumbnail = MakeShareable(new FAssetThumbnail(Value.AssetData, ThumbnailSize.X, ThumbnailSize.Y,
                                                       FDungeonEditorThumbnailPool::Get()));
}

bool SGraphNode_DungeonActor::GetValue(FObjectOrAssetData& OutValue) const {
    // Potentially accessing the value while garbage collecting or saving the package could trigger a crash.
    // so we fail to get the value when that is occurring.
    if (GIsSavingPackage || IsGarbageCollecting()) {
        return false;
    }

    //UObject* Object = EdActorNode->GetNodeAssetObject();
    UObject* Object = EdActorNode->GetThumbnailAssetObject();
    OutValue = FObjectOrAssetData(Object);

    bool foundObject = (Object == nullptr);
    return foundObject;
}

void SGraphNode_DungeonActor::UpdateGraphNode() {
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

    IndexOverlay = SNew(SMeshGraphNodeIndex)
		.ToolTipText(this, &SGraphNode_DungeonActor::GetIndexTooltipText)
		.Visibility(this, &SGraphNode_DungeonActor::GetIndexVisibility)
		.Text(this, &SGraphNode_DungeonActor::GetIndexText)
		.OnHoverStateChanged(this, &SGraphNode_DungeonActor::OnIndexHoverStateChanged)
		.OnGetIndexColor(this, &SGraphNode_DungeonActor::GetIndexColor);

    this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
    this->GetOrAddSlot(ENodeZone::Center)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
    [
        SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor(this, &SGraphNode_DungeonActor::GetBorderBackgroundColor)
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


            // THUMBNAIL AREA
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
                [
                    SNew(SVerticalBox)
                    // Thumbnail Slot
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    [
                        SNew(SBox)
								.WidthOverride(ThumbnailSize.X)
								.HeightOverride(ThumbnailSize.Y)
                        [
                            AssetThumbnail->MakeThumbnailWidget()
                        ]
                    ]
                ]
            ]
        ]
    ];

    CreatePinWidgets();
}

static UEdGraphNode_DungeonBase* GetParentNode(UEdGraphNode* GraphNode) {
    UEdGraphNode_DungeonBase* StateNode = CastChecked<UEdGraphNode_DungeonBase>(GraphNode);
    UEdGraphPin* MyInputPin = StateNode->GetInputPin();
    UEdGraphPin* MyParentOutputPin = nullptr;
    if (MyInputPin != nullptr && MyInputPin->LinkedTo.Num() > 0) {
        MyParentOutputPin = MyInputPin->LinkedTo[0];
        if (MyParentOutputPin != nullptr) {
            if (MyParentOutputPin->GetOwningNode() != nullptr) {
                return CastChecked<UEdGraphNode_DungeonBase>(MyParentOutputPin->GetOwningNode());
            }
        }
    }

    return nullptr;
}

void SGraphNode_DungeonActor::OnIndexHoverStateChanged(bool bHovered) {
    UEdGraphNode_DungeonBase* ParentNode = GetParentNode(GraphNode);
    if (ParentNode != nullptr) {
        ParentNode->bHighlightChildNodeIndices = bHovered;
    }
}

FSlateColor SGraphNode_DungeonActor::GetIndexColor(bool bHovered) const {
    UEdGraphNode_DungeonBase* ParentNode = GetParentNode(GraphNode);
    const bool bHighlightHover = bHovered || (ParentNode && ParentNode->bHighlightChildNodeIndices);

    static const FName NodeHoveredColor("BTEditor.Graph.BTNode.Index.HoveredColor");
    static const FName NodeDefaultColor("BTEditor.Graph.BTNode.Index.Color");

    return bHovered
               ? FEditorStyle::Get().GetSlateColor(NodeHoveredColor)
               : FEditorStyle::Get().GetSlateColor(NodeDefaultColor);
}

EVisibility SGraphNode_DungeonActor::GetIndexVisibility() const {
    return EVisibility::Visible;
}

FText SGraphNode_DungeonActor::GetIndexText() const {
    UEdGraphNode_DungeonActorBase* ActorNode = CastChecked<UEdGraphNode_DungeonActorBase>(GraphNode);
    return FText::AsNumber(ActorNode->ExecutionOrder);
}

FText SGraphNode_DungeonActor::GetIndexTooltipText() const {
    if (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != nullptr) {
        return LOCTEXT("ExecutionIndexTooltip", "Execution index: this shows the order in which nodes are executed.");
    }
    return LOCTEXT("ChildIndexTooltip", "Child index: this shows the order in which child nodes are executed.");
}

void SGraphNode_DungeonActor::CreatePinWidgets() {
    UEdGraphNode_DungeonActorBase* DungeonNode = CastChecked<UEdGraphNode_DungeonActorBase>(GraphNode);

    {
        UEdGraphPin* CurPin = DungeonNode->GetOutputPin();
        TSharedPtr<SGraphPin> NewPin = SNew(SDungeonOutputPin, CurPin);
        NewPin->SetIsEditable(IsEditable);
        this->AddPin(NewPin.ToSharedRef());
        OutputPins.Add(NewPin.ToSharedRef());
    }
    {
        UEdGraphPin* CurPin = DungeonNode->GetInputPin();
        TSharedPtr<SGraphPin> NewPin = SNew(SDungeonOutputPin, CurPin);
        NewPin->SetIsEditable(IsEditable);
        this->AddPin(NewPin.ToSharedRef());
        InputPins.Add(NewPin.ToSharedRef());
    }
}

void SGraphNode_DungeonActor::AddPin(const TSharedRef<SGraphPin>& PinToAdd) {
    /*
    PinToAdd->SetOwner(SharedThis(this));
    RightNodeBox->AddSlot()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        .FillHeight(1.0f)
        [
            PinToAdd
        ];
        */
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


TArray<FOverlayWidgetInfo>
SGraphNode_DungeonActor::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const {
    TArray<FOverlayWidgetInfo> Widgets;
    check(IndexOverlay.IsValid());

    FVector2D Origin(0.0f, 0.0f);

    FOverlayWidgetInfo Overlay(IndexOverlay);
    Overlay.OverlayOffset = FVector2D(WidgetSize.X - (IndexOverlay->GetDesiredSize().X * 0.5f), Origin.Y);
    Widgets.Add(Overlay);


    return Widgets;
}


void SGraphNode_DungeonActor::RebuildExecutionOrder() {
    UEdGraphNode_DungeonBase* Parent = GetParentNode(GraphNode);
    if (Parent) {
        Parent->UpdateChildExecutionOrder();
    }
}


void SGraphNode_DungeonActor::MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter) {
    RebuildExecutionOrder();
    return SGraphNode::MoveTo(NewPosition, NodeFilter);
}

#undef LOCTEXT_NAMESPACE

