//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdAbstractGraph.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdTilemap.h"
#include "Core/Editors/FlowEditor/DomainMediators/FlowDomainEdMediator.h"

class SFlowPreview3DViewport;

class FAbstractGraphTilemapDomainMediator
    : public IFlowDomainEdMediator
    , public FFlowDomainEdAbstractGraph::IMediator
    , public FFlowDomainEdTilemap::IMediator
    , public TSharedFromThis<FAbstractGraphTilemapDomainMediator>
{
public:
    void Initialize(TSharedPtr<FFlowDomainEdAbstractGraph> InDomainAbstractGraph,
        TSharedPtr<FFlowDomainEdTilemap> InDomainTilemap, TSharedPtr<SFlowPreview3DViewport> InPreviewViewport);

    //~Begin FFlowDomainEdAbstractGraph::IMediator Interface
    virtual void OnAbstractNodeSelectionChanged(TArray<UGridFlowAbstractEdGraphNode*> EdNodes) override;
    virtual void OnAbstractItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) override;
    //~End FFlowDomainEdAbstractGraph::IMediator Interface
    
    //~Begin FFlowDomainEdTilemap::IMediator Interface
    virtual void OnTilemapItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) override;
    virtual void GetAllTilemapItems(FFlowExecNodeStatePtr State, TArray<UFlowGraphItem*>& OutItems) override;
    virtual void UpdateTilemapChunkSelection(const FVector& InChunkCoords) override;
    virtual void OnTilemapCellClicked(const FIntPoint& InTileCoords, bool bDoubleClicked) override;
    virtual void FocusViewportOnTileCoord(const FVector& InCoord) override;
    //~End FFlowDomainEdTilemap::IMediator Interface

private:
    TWeakPtr<FFlowDomainEdAbstractGraph> DomainAbstractGraph;
    TWeakPtr<FFlowDomainEdTilemap> DomainTilemap;
    TWeakPtr<SFlowPreview3DViewport> PreviewViewport;
    
    bool bIgnoreTilemapPreviewRebuildRequest = false;
};

