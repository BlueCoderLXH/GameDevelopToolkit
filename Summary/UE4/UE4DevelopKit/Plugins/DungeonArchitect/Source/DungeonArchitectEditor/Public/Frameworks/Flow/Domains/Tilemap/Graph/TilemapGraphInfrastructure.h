//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Common/Widgets/SGridFlowItemOverlayDelegates.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"

#include "ConnectionDrawingPolicy.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "EdGraphUtilities.h"
#include "SGraphNode.h"
#include "TilemapGraphInfrastructure.generated.h"

class UTexture;
class UGridFlowTilemap;
struct FGridFlowTilemapRendererSettings;

DECLARE_DELEGATE_TwoParams(FGridFlowTilemapNodeCellClicked, const FIntPoint& /* TileCoords */,
                           bool /* bDoubleClicked */);

/////////////////// Graph /////////////////// 
UCLASS()
class DUNGEONARCHITECTEDITOR_API UGridFlowTilemapEdGraph : public UEdGraph {
    GENERATED_UCLASS_BODY()

public:
    void Initialize();
    void GeneratePreviewTexture(UGridFlowTilemap* InTilemap, const FGridFlowTilemapRendererSettings& InRenderSettings,
                                const TArray<UFlowGraphItem*>& InItems);
    UTexture* GetPreviewTexture();

public:
    UPROPERTY()
    class UGridFlowTilemapEdGraphNode* PreviewNode;

    FGridFlowTilemapNodeCellClicked OnCellClicked;
    FGridFlowItemWidgetEvent OnItemWidgetClicked;
    FGuid SelectedItemId;
};

/////////////////// Graph Node /////////////////// 

UCLASS()
class DUNGEONARCHITECTEDITOR_API UGridFlowTilemapEdGraphNode : public UEdGraphNode {
    GENERATED_BODY()
public:

    void SetItemInfo(const TMap<FGuid, FGridFlowTilemapCoord>& InTileItems, const TMap<FGuid, TWeakObjectPtr<UFlowGraphItem>>& InItemList);
    void ClearItemInfo();

public:
    UPROPERTY()
    UTexture* PreviewTexture = nullptr;

    TMap<FGuid, FGridFlowTilemapCoord> TileItems;
    TMap<FGuid, TWeakObjectPtr<UFlowGraphItem>> ItemList;
};

/////////////////// Graph Schema /////////////////// 

UCLASS()
class DUNGEONARCHITECTEDITOR_API UGridFlowTilemapEdGraphSchema : public UEdGraphSchema {
    GENERATED_BODY()
public:
    // Begin EdGraphSchema interface
    virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID,
                                                                          float InZoomFactor, const FSlateRect& InClippingRect,
                                                                          class FSlateWindowElementList& InDrawElements,
                                                                          class UEdGraph* InGraphObj) const override;
    // End EdGraphSchema interface
};

/////////////////// Link Drawing Policy /////////////////// 
class DUNGEONARCHITECTEDITOR_API FGridFlowTilemapConnectionDrawingPolicy : public FConnectionDrawingPolicy {
public:
    FGridFlowTilemapConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor,
                                            const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements,
                                            UEdGraph* InGraphObj);
    virtual void Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& InArrangedNodes) override;
    virtual void DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom,
                                     const FConnectionParams& Params) override;
    virtual void DrawSplineWithArrow(const FVector2D& StartPoint, const FVector2D& EndPoint,
                                     const FConnectionParams& Params) override;
    virtual void DrawConnection(int32 LayerId, const FVector2D& Start, const FVector2D& End,
                                const FConnectionParams& Params) override;

private:
    UEdGraph* GraphObj;
    void DrawItemReferenceLink(const FVector2D& Src, const FVector2D& Dest, const FConnectionParams& Params);
};

/////////////////// Graph Handler ///////////////////

DECLARE_DELEGATE_OneParam(FTilemapGraphNodeDelegate, UEdGraphNode*);
DECLARE_DELEGATE_OneParam(FTilemapGraphSelectedNodeChangedDelegate, const TSet<class UObject*>&);

class FGridFlowTilemapGraphHandler : public TSharedFromThis<FGridFlowTilemapGraphHandler> {
public:
    void Bind();
    void SetGraphEditor(TSharedPtr<SGraphEditor> InGraphEditor);
    void SetPropertyEditor(TWeakPtr<IDetailsView> InPropertyEditor);

private:

    /** Select every node in the graph */
    void SelectAllNodes();
    /** Whether we can select every node */
    bool CanSelectAllNodes() const;

    /** Deletes all the selected nodes */
    void DeleteSelectedNodes();

    bool CanDeleteNode(class UEdGraphNode* Node);

    /** Delete an array of Material Graph Nodes and their corresponding expressions/comments */
    void DeleteNodes(const TArray<class UEdGraphNode*>& NodesToDelete);

    /** Delete only the currently selected nodes that can be duplicated */
    void DeleteSelectedDuplicatableNodes();

    /** Whether we are able to delete the currently selected nodes */
    bool CanDeleteNodes() const;

    /** Copy the currently selected nodes */
    void CopySelectedNodes();
    /** Whether we are able to copy the currently selected nodes */
    bool CanCopyNodes() const;

    /** Paste the contents of the clipboard */
    void PasteNodes();
    bool CanPasteNodes() const;
    void PasteNodesHere(const FVector2D& Location);

    /** Cut the currently selected nodes */
    void CutSelectedNodes();
    /** Whether we are able to cut the currently selected nodes */
    bool CanCutNodes() const;

    /** Duplicate the currently selected nodes */
    void DuplicateNodes();
    /** Whether we are able to duplicate the currently selected nodes */
    bool CanDuplicateNodes() const;

    /** Called when the selection changes in the GraphEditor */
    void HandleSelectedNodesChanged(const TSet<class UObject*>& NewSelection);

    /** Called when a node is double clicked */
    void HandleNodeDoubleClicked(class UEdGraphNode* Node);

public:
    SGraphEditor::FGraphEditorEvents GraphEvents;
    TSharedPtr<FUICommandList> GraphEditorCommands;

    FTilemapGraphNodeDelegate OnNodeDoubleClicked;
    FTilemapGraphSelectedNodeChangedDelegate OnNodeSelectionChanged;

private:
    TSharedPtr<SGraphEditor> GraphEditor;
    TWeakPtr<IDetailsView> PropertyEditor;
};


/////////////////// Node Widget /////////////////// 

/**
 * Implements the message interaction graph panel.
 */
class DUNGEONARCHITECTEDITOR_API SGridFlowTilemapGraphNode : public SGraphNode {
public:
    SLATE_BEGIN_ARGS(SGridFlowTilemapGraphNode) {}
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, UGridFlowTilemapEdGraphNode* InNode);

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    virtual TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

    // End of SGraphNode interface

    struct FNodeItemInfo {
        FGuid ItemId;
        TWeakObjectPtr<UFlowGraphItem> Item;
        TSharedPtr<class SGridFlowItemOverlay> Widget;
        FGridFlowTilemapCoord TileCoord;
        FVector2D RenderOffset;
    };

    const TMap<FGuid, FNodeItemInfo>& GetItemInfo() const { return ItemInfoList; }
    UGridFlowTilemapEdGraphNode* GetTilemapNode() const { return TilemapNode; }
    void OnItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked);

protected:
    FSlateColor GetBorderBackgroundColor() const;
    bool IsItemSelected(FGuid InItemId) const;
    void HandleMouseClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, bool bDoubleClick);

protected:
    UGridFlowTilemapEdGraphNode* TilemapNode = nullptr;
    FSlateBrush TextureBrush;
    FVector2D TextureSize = FVector2D::ZeroVector;
    TMap<FGuid, FNodeItemInfo> ItemInfoList;
};

/////////////////// Node Widget Factory /////////////////// 

class DUNGEONARCHITECTEDITOR_API FGridFlowTilemapGraphPanelNodeFactory : public FGraphPanelNodeFactory {
public:
    virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override;
};

