//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/Graph/TilemapGraphInfrastructure.h"

#include "Frameworks/Flow/Common/Widgets/SGridFlowItemOverlay.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapRenderer.h"

#include "EdGraph/EdGraph.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Framework/Commands/GenericCommands.h"
#include "IDetailsView.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"

DEFINE_LOG_CATEGORY_STATIC(LogTilemapEdGraph, Log, All);

struct FGFTilemapNodeItemInfo {
    FGuid ItemId;
    TWeakObjectPtr<UFlowGraphItem> Item;
    TSharedPtr<class SGridFlowItemOverlay> Widget;
    FGridFlowTilemapCoord TileCoord;
    FVector2D RenderOffset;
};


namespace {
    const int32 DAGF_TileSize = 10; // TODO: Grab from the tilemap
    const int32 DAGF_TilePadding = 10;
}

/////////////////// Graph /////////////////// 
UGridFlowTilemapEdGraph::UGridFlowTilemapEdGraph(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Schema = UGridFlowTilemapEdGraphSchema::StaticClass();
}

void UGridFlowTilemapEdGraph::Initialize() {
    FGraphNodeCreator<UGridFlowTilemapEdGraphNode> NodeCreator(*this);
    PreviewNode = NodeCreator.CreateNode(false);
    NodeCreator.Finalize();
}

void UGridFlowTilemapEdGraph::GeneratePreviewTexture(UGridFlowTilemap* InTilemap,
                                                     const FGridFlowTilemapRendererSettings& InRenderSettings,
                                                     const TArray<UFlowGraphItem*>& InItems) {
    if (!PreviewNode) return;

    // Release the old tilemap Render Texture
    if (UTextureRenderTarget2D* RTT = Cast<UTextureRenderTarget2D>(PreviewNode->PreviewTexture)) {
        UKismetRenderingLibrary::ReleaseRenderTarget2D(RTT);
        PreviewNode->PreviewTexture = nullptr;
    }

    if (InTilemap) {
        PreviewNode->PreviewTexture = FGridFlowTilemapRenderer::Create(InTilemap, InRenderSettings);

        // Update the items
        TMap<FGuid, FGridFlowTilemapCoord> TileItems;
        for (const FGridFlowTilemapCell& Cell : InTilemap->GetCells()) {
            if (Cell.bHasItem) {
                TileItems.Add(Cell.ItemId, FGridFlowTilemapCoord(Cell.TileCoord));
            }
        }
        for (const FGridFlowTilemapEdge& Edge : InTilemap->GetEdgesH()) {
            if (Edge.bHasItem) {
                TileItems.Add(Edge.ItemId, Edge.EdgeCoord);
            }
        }
        for (const FGridFlowTilemapEdge& Edge : InTilemap->GetEdgesV()) {
            if (Edge.bHasItem) {
                TileItems.Add(Edge.ItemId, Edge.EdgeCoord);
            }
        }
        TMap<FGuid, TWeakObjectPtr<UFlowGraphItem>> ItemList;
        for (UFlowGraphItem* Item : InItems) {
            TWeakObjectPtr<UFlowGraphItem>& ItemRef = ItemList.FindOrAdd(Item->ItemId);
            ItemRef = Item;
        }
        PreviewNode->SetItemInfo(TileItems, ItemList);
    }
    else {
        PreviewNode->PreviewTexture = nullptr;
        PreviewNode->ClearItemInfo();
    }

    // Update the preview node's location to align the center with the origin
    UTexture* Texture = PreviewNode->PreviewTexture;
    float Width = 0;
    float Height = 0;
    if (Texture) {
        Width = Texture->GetSurfaceWidth();
        Height = Texture->GetSurfaceHeight();
    }
    PreviewNode->NodePosX = -Width * 0.5f;
    PreviewNode->NodePosY = -Height * 0.5f;

    NotifyGraphChanged();
}

UTexture* UGridFlowTilemapEdGraph::GetPreviewTexture() {
    return PreviewNode ? PreviewNode->PreviewTexture : nullptr;
}

/////////////////// Graph Schema /////////////////// 
FConnectionDrawingPolicy* UGridFlowTilemapEdGraphSchema::CreateConnectionDrawingPolicy(
    int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect,
    class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const {
    return new FGridFlowTilemapConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect,
                                                       InDrawElements, InGraphObj);
}

/////////////////// Link Drawing Policy /////////////////// 
FGridFlowTilemapConnectionDrawingPolicy::FGridFlowTilemapConnectionDrawingPolicy(
    int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect,
    FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
    : FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
      , GraphObj(InGraphObj) {
}

void FGridFlowTilemapConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
                                                   FArrangedChildren& InArrangedNodes) {
    if (InArrangedNodes.Num() == 0) {
        return;
    }

    FArrangedWidget& CurWidget = InArrangedNodes[0];
    TSharedRef<SGridFlowTilemapGraphNode> TilemapWidget = StaticCastSharedRef<SGridFlowTilemapGraphNode>(
        CurWidget.Widget);

    const TMap<FGuid, SGridFlowTilemapGraphNode::FNodeItemInfo>& ItemInfoList = TilemapWidget->GetItemInfo();

    FVector2D NodePadding(DAGF_TilePadding, DAGF_TilePadding);
    FVector2D Scale = FVector2D(1, 1);
    UGridFlowTilemapEdGraphNode* TilemapNode = TilemapWidget->GetTilemapNode();
    if (TilemapNode && TilemapNode->PreviewTexture) {
        float Width = TilemapNode->PreviewTexture->GetSurfaceWidth();
        float Height = TilemapNode->PreviewTexture->GetSurfaceHeight();
        FVector2D TextureSize(Width, Height);
        FVector2D DefaultSize = TextureSize + NodePadding * 2;
        FVector2D DrawSize = CurWidget.Geometry.GetDrawSize();
        Scale = DrawSize / DefaultSize;
    }

    for (auto& Entry : ItemInfoList) {
        const FGuid& ItemId = Entry.Key;
        const SGridFlowTilemapGraphNode::FNodeItemInfo& ItemInfo = Entry.Value;
        if (!ItemInfo.Item.IsValid()) continue;
        
        for (const FGuid& RefItemId : ItemInfo.Item->ReferencedItemIds) {
            const SGridFlowTilemapGraphNode::FNodeItemInfo* RefItem = ItemInfoList.Find(RefItemId);
            if (RefItem) {
                FVector2D Src = CurWidget.Geometry.AbsolutePosition + ItemInfo.RenderOffset * Scale;
                FVector2D Dest = CurWidget.Geometry.AbsolutePosition + RefItem->RenderOffset * Scale;
                FVector2D Direction = Dest - Src;
                Direction.Normalize();
                Dest -= Direction * (RefItem->Widget->GetWidgetRadius() - 1) * Scale;

                FConnectionParams Params;
                Params.WireColor = FLinearColor::Red;
                Params.WireThickness = 1.0f;
                DrawItemReferenceLink(Src, Dest, Params);
            }
        }
    }
}

void FGridFlowTilemapConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom,
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

void FGridFlowTilemapConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D& StartPoint,
                                                                  const FVector2D& EndPoint,
                                                                  const FConnectionParams& Params) {
    DrawItemReferenceLink(StartPoint, EndPoint, Params);
}

void FGridFlowTilemapConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FVector2D& Start,
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

void FGridFlowTilemapConnectionDrawingPolicy::DrawItemReferenceLink(const FVector2D& Src, const FVector2D& Dest,
                                                                    const FConnectionParams& Params) {
    const FVector2D ArrowHeadRadius = ArrowRadius;
    const FSlateBrush* ArrowHeadImage = ArrowImage;

    const FVector2D StartAnchorPoint = Src;
    const FVector2D EndAnchorPoint = Dest;
    const FVector2D DeltaPos = EndAnchorPoint - StartAnchorPoint;
    const FVector2D UnitDelta = DeltaPos.GetSafeNormal();
    const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

    // Come up with the final start/end points
    const FVector2D LengthBias = ArrowHeadRadius.X * UnitDelta;

    const FVector2D StartPoint = StartAnchorPoint;
    FVector2D EndPoint = EndAnchorPoint;

    EndPoint -= LengthBias;

    DrawConnection(ArrowLayerID, StartPoint, EndPoint, Params);

    FVector2D ArrowDrawPos = (EndAnchorPoint - LengthBias) - ArrowHeadRadius;
    const float AngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

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

/////////////////// Graph Handler ///////////////////

void FGridFlowTilemapGraphHandler::Bind() {
    GraphEditorCommands = MakeShareable(new FUICommandList);
    GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
                                   FExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::SelectAllNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::CanSelectAllNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
                                   FExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::DeleteSelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::CanDeleteNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
                                   FExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::CopySelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::CanCopyNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
                                   FExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::PasteNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::CanPasteNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
                                   FExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::CutSelectedNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::CanCutNodes)
    );

    GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
                                   FExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::DuplicateNodes),
                                   FCanExecuteAction::CreateSP(this, &FGridFlowTilemapGraphHandler::CanDuplicateNodes)
    );

    GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(
        this, &FGridFlowTilemapGraphHandler::HandleSelectedNodesChanged);
    GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(
        this, &FGridFlowTilemapGraphHandler::HandleNodeDoubleClicked);
}

void FGridFlowTilemapGraphHandler::SetGraphEditor(TSharedPtr<SGraphEditor> InGraphEditor) {
    this->GraphEditor = InGraphEditor;
}

void FGridFlowTilemapGraphHandler::SetPropertyEditor(TWeakPtr<IDetailsView> InPropertyEditor) {
    PropertyEditor = InPropertyEditor;
}

void FGridFlowTilemapGraphHandler::SelectAllNodes() {
    if (!GraphEditor.IsValid()) return;
    GraphEditor->SelectAllNodes();
}

bool FGridFlowTilemapGraphHandler::CanSelectAllNodes() const {
    return GraphEditor.IsValid();
}

void FGridFlowTilemapGraphHandler::DeleteSelectedNodes() {
}

bool FGridFlowTilemapGraphHandler::CanDeleteNode(class UEdGraphNode* Node) {
    return false;
}

void FGridFlowTilemapGraphHandler::DeleteNodes(const TArray<class UEdGraphNode*>& NodesToDelete) {
}

void FGridFlowTilemapGraphHandler::DeleteSelectedDuplicatableNodes() {
}

bool FGridFlowTilemapGraphHandler::CanDeleteNodes() const {
    return false;
}

void FGridFlowTilemapGraphHandler::CopySelectedNodes() {
}

bool FGridFlowTilemapGraphHandler::CanCopyNodes() const {
    return false;
}

void FGridFlowTilemapGraphHandler::PasteNodes() {

}

bool FGridFlowTilemapGraphHandler::CanPasteNodes() const {
    return false;
}

void FGridFlowTilemapGraphHandler::PasteNodesHere(const FVector2D& Location) {
}

void FGridFlowTilemapGraphHandler::CutSelectedNodes() {
}

bool FGridFlowTilemapGraphHandler::CanCutNodes() const {
    return false;
}

void FGridFlowTilemapGraphHandler::DuplicateNodes() {
}

bool FGridFlowTilemapGraphHandler::CanDuplicateNodes() const {
    return false;
}

void FGridFlowTilemapGraphHandler::HandleSelectedNodesChanged(const TSet<class UObject*>& NewSelection) {
    if (PropertyEditor.IsValid()) {
        TSharedPtr<IDetailsView> DetailsView = PropertyEditor.Pin();
        DetailsView->SetObjects(NewSelection.Array());
    }

    OnNodeSelectionChanged.ExecuteIfBound(NewSelection);
}

void FGridFlowTilemapGraphHandler::HandleNodeDoubleClicked(class UEdGraphNode* Node) {
    OnNodeDoubleClicked.ExecuteIfBound(Node);
}

/////////////////// Node Widget /////////////////// 

void SGridFlowTilemapGraphNode::Construct(const FArguments& InArgs, UGridFlowTilemapEdGraphNode* InNode) {
    GraphNode = InNode;
    TilemapNode = InNode;
    UpdateGraphNode();
}

void SGridFlowTilemapGraphNode::UpdateGraphNode() {
    InputPins.Empty();
    OutputPins.Empty();

    // Reset variables that are going to be exposed, in case we are refreshing an already setup node.
    RightNodeBox.Reset();
    LeftNodeBox.Reset();

    static const FVector2D NodePadding(DAGF_TilePadding, DAGF_TilePadding);

    // Create the item widgets
    {
        ItemInfoList.Reset();

        for (auto& Entry : TilemapNode->ItemList) {
            TWeakObjectPtr<UFlowGraphItem> ItemPtr = Entry.Value;
            if (!ItemPtr.IsValid()) continue;
            UFlowGraphItem* Item = ItemPtr.Get();
            TSharedPtr<SGridFlowItemOverlay> ItemWidget = SNew(SGridFlowItemOverlay, Item)
                .Selected(this, &SGridFlowTilemapGraphNode::IsItemSelected, Item->ItemId);
            ItemWidget->GetOnMousePressed().BindRaw(this, &SGridFlowTilemapGraphNode::OnItemWidgetClicked);

            FGridFlowTilemapCoord* ItemCoordPtr = TilemapNode->TileItems.Find(Item->ItemId);
            if (ItemCoordPtr) {
                FGridFlowTilemapCoord Coord = *ItemCoordPtr;
                FVector2D RenderOffset = NodePadding + FVector2D(Coord.Coord.X, Coord.Coord.Y) * DAGF_TileSize;
                if (!Coord.bIsEdgeCoord) {
                    RenderOffset += FVector2D(0.5f, 0.5f) * DAGF_TileSize;
                }
                else {
                    if (Coord.bHorizontalEdge) {
                        RenderOffset += FVector2D(0.5f, 0) * DAGF_TileSize;
                    }
                    else {
                        RenderOffset += FVector2D(0, 0.5f) * DAGF_TileSize;
                    }
                }

                FNodeItemInfo ItemInfo;
                ItemInfo.ItemId = Item->ItemId;
                ItemInfo.Item = Item;
                ItemInfo.TileCoord = Coord;
                ItemInfo.RenderOffset = RenderOffset;
                ItemInfo.Widget = ItemWidget;
                ItemInfoList.Add(ItemInfo.ItemId, ItemInfo);
            }
        }
    }

    FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
    TextureSize = FVector2D::ZeroVector;
    if (TilemapNode && TilemapNode->PreviewTexture) {
        TextureSize.X = TilemapNode->PreviewTexture->GetSurfaceWidth();
        TextureSize.Y = TilemapNode->PreviewTexture->GetSurfaceHeight();
        TextureBrush.SetResourceObject(TilemapNode->PreviewTexture);
        TextureBrush.ImageSize = TextureSize;
    }
    else {
        TextureBrush.SetResourceObject(nullptr);
    }

    this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
    this->GetOrAddSlot(ENodeZone::Center)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
    [
        SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor(this, &SGridFlowTilemapGraphNode::GetBorderBackgroundColor)
        [
            SNew(SOverlay)
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
                    SNew(SBox)
						.WidthOverride(TextureSize.X)
						.HeightOverride(TextureSize.Y)
                    [
                        SNew(SImage)
                        .Image(&TextureBrush)
                    ]
                ]
            ]
        ]
    ];
}

TArray<FOverlayWidgetInfo> SGridFlowTilemapGraphNode::GetOverlayWidgets(
    bool bSelected, const FVector2D& WidgetSize) const {
    TArray<FOverlayWidgetInfo> Overlays;
    FVector2D Scale = (TextureSize + DAGF_TilePadding * 2) / WidgetSize;

    for (auto& Entry : ItemInfoList) {
        FGuid ItemId = Entry.Key;
        const FNodeItemInfo& ItemInfo = Entry.Value;
        TSharedPtr<SGridFlowItemOverlay> Widget = ItemInfo.Widget;
        if (Widget.IsValid()) {
            float Radius = Widget->GetWidgetRadius();
            FVector2D Offset = ItemInfo.RenderOffset - Widget->GetDesiredSize() * 0.5f + FVector2D(2, 3);
            FOverlayWidgetInfo OverlayInfo;
            OverlayInfo.Widget = Widget;
            OverlayInfo.OverlayOffset = Offset;
            Overlays.Add(OverlayInfo);
        }
    }

    return Overlays;
}

FReply SGridFlowTilemapGraphNode::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    HandleMouseClick(MyGeometry, MouseEvent, false);
    return SGraphNode::OnMouseButtonDoubleClick(MyGeometry, MouseEvent);
}

FReply SGridFlowTilemapGraphNode::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry,
                                                           const FPointerEvent& InMouseEvent) {
    HandleMouseClick(InMyGeometry, InMouseEvent, true);
    return SGraphNode::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

void SGridFlowTilemapGraphNode::HandleMouseClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent,
                                                 bool bDoubleClick) {
    UGridFlowTilemapEdGraph* Graph = Cast<UGridFlowTilemapEdGraph>(GetNodeObj()->GetGraph());

    if (Graph && Graph->OnCellClicked.IsBound()) {
        FVector2D MousePosition = MouseEvent.GetScreenSpacePosition();
        FVector2D WidgetPosition = MyGeometry.GetAbsolutePosition();
        FVector2D Delta = MousePosition - WidgetPosition;
        Delta /= MyGeometry.Scale;

        Delta -= FVector2D(DAGF_TilePadding, DAGF_TilePadding);
        int32 TileX = FMath::FloorToInt(Delta.X / DAGF_TileSize);
        int32 TileY = FMath::FloorToInt(Delta.Y / DAGF_TileSize);

        Graph->OnCellClicked.Execute(FIntPoint(TileX, TileY), bDoubleClick);
    }
}


void SGridFlowTilemapGraphNode::OnItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) {
    UGridFlowTilemapEdGraph* Graph = Cast<UGridFlowTilemapEdGraph>(GetNodeObj()->GetGraph());
    if (Graph) {
        //  Invoke a tile click event, for the tile under this item
        {
            UGridFlowTilemapEdGraphNode* EdNode = Cast<UGridFlowTilemapEdGraphNode>(GetNodeObj());
            FGridFlowTilemapCoord* SearchResult = EdNode->TileItems.Find(InItemId);
            if (SearchResult) {
                FGridFlowTilemapCoord TileCoord = *SearchResult;

                Graph->OnCellClicked.Execute(TileCoord.Coord, bDoubleClicked);
            }
        }

        Graph->OnItemWidgetClicked.ExecuteIfBound(InItemId, bDoubleClicked);
    }
}

bool SGridFlowTilemapGraphNode::IsItemSelected(FGuid InItemId) const {
    if (UGridFlowTilemapEdGraph* Graph = Cast<UGridFlowTilemapEdGraph>(GetNodeObj()->GetGraph())) {
        return Graph->SelectedItemId == InItemId;
    }
    return false;
}

FSlateColor SGridFlowTilemapGraphNode::GetBorderBackgroundColor() const {
    return FLinearColor(0.08f, 0.08f, 0.08f);
}

/////////////////// Node Widget Factory /////////////////// 

TSharedPtr<SGraphNode> FGridFlowTilemapGraphPanelNodeFactory::CreateNode(UEdGraphNode* Node) const {
    if (UGridFlowTilemapEdGraphNode* TilemapNode = Cast<UGridFlowTilemapEdGraphNode>(Node)) {
        TSharedPtr<SGridFlowTilemapGraphNode> SNode = SNew(SGridFlowTilemapGraphNode, TilemapNode);
        return SNode;
    }

    return nullptr;
}

void UGridFlowTilemapEdGraphNode::SetItemInfo(const TMap<FGuid, FGridFlowTilemapCoord>& InTileItems,
                                              const TMap<FGuid, TWeakObjectPtr<UFlowGraphItem>>& InItemList) {
    TileItems = InTileItems;
    ItemList = InItemList;
}

void UGridFlowTilemapEdGraphNode::ClearItemInfo() {
    TileItems.Reset();
    ItemList.Reset();
}

