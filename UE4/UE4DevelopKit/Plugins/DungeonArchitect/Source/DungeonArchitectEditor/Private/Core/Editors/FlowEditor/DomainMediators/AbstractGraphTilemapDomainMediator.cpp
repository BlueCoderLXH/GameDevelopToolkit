//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/DomainMediators/AbstractGraphTilemapDomainMediator.h"

#include "Builders/GridFlow/GridFlowConfig.h"
#include "Builders/GridFlow/GridFlowQuery.h"
#include "Core/Dungeon.h"
#include "Core/Editors/FlowEditor/Panels/Viewport/SFlowPreview3DViewport.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodes.h"

struct FGridFlowTilemapCell;

void FAbstractGraphTilemapDomainMediator::Initialize(TSharedPtr<FFlowDomainEdAbstractGraph> InDomainAbstractGraph,
                                                     TSharedPtr<FFlowDomainEdTilemap> InDomainTilemap, TSharedPtr<SFlowPreview3DViewport> InPreviewViewport)
{
    DomainAbstractGraph = InDomainAbstractGraph;
    DomainTilemap = InDomainTilemap;
    PreviewViewport = InPreviewViewport;

    if (InDomainAbstractGraph.IsValid()) {
        InDomainAbstractGraph->SetMediator(SharedThis(this));
    }
    if (InDomainTilemap.IsValid()) {
        InDomainTilemap->SetMediator(SharedThis(this));
    }
}

void FAbstractGraphTilemapDomainMediator::OnAbstractNodeSelectionChanged(TArray<UGridFlowAbstractEdGraphNode*> EdNodes) {
    bool bChunkSelected;
    FVector SelectedChunkCoord;
    if (EdNodes.Num() == 1) {
        SelectedChunkCoord = EdNodes[0]->ScriptNode->Coord;
        bChunkSelected = true;
    }
    else {
        bChunkSelected = false;
        SelectedChunkCoord = FVector::ZeroVector;
    }
    
    const TSharedPtr<FFlowDomainEdTilemap> DomainTilemapPtr = DomainTilemap.Pin();
    if (DomainTilemapPtr.IsValid()) {
        DomainTilemapPtr->SelectChunk(SelectedChunkCoord, bChunkSelected);
        if (!bIgnoreTilemapPreviewRebuildRequest) {
            DomainTilemapPtr->RefreshPreviewTilemap();
        }
    }
}

void FAbstractGraphTilemapDomainMediator::OnAbstractItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) {
    const TSharedPtr<FFlowDomainEdTilemap> DomainTilemapPtr = DomainTilemap.Pin();
    if (DomainTilemapPtr.IsValid()) {
        DomainTilemapPtr->SelectItem(InItemId);
    }
}

void FAbstractGraphTilemapDomainMediator::OnTilemapItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) {
    const TSharedPtr<FFlowDomainEdAbstractGraph> DomainAbstractGraphPtr = DomainAbstractGraph.Pin();
    if (DomainAbstractGraphPtr.IsValid()) {
        DomainAbstractGraphPtr->SelectItem(InItemId);
    }
}

void FAbstractGraphTilemapDomainMediator::GetAllTilemapItems(FFlowExecNodeStatePtr State, TArray<UFlowGraphItem*>& OutItems) {
    const TSharedPtr<FFlowDomainEdAbstractGraph> DomainAbstractGraphPtr = DomainAbstractGraph.Pin();
    if (DomainAbstractGraphPtr.IsValid()) {
        DomainAbstractGraphPtr->GetAllItems(State, OutItems);
    }
}

void FAbstractGraphTilemapDomainMediator::UpdateTilemapChunkSelection(const FVector& InChunkCoords) {
    const TSharedPtr<FFlowDomainEdAbstractGraph> DomainAbstractGraphPtr = DomainAbstractGraph.Pin();
    if (DomainAbstractGraphPtr.IsValid()) {
        bIgnoreTilemapPreviewRebuildRequest = true;
        DomainAbstractGraphPtr->ClearAllSelections();
        bIgnoreTilemapPreviewRebuildRequest = false;
        DomainAbstractGraphPtr->SelectNode(InChunkCoords, true);
    }
}

void FAbstractGraphTilemapDomainMediator::OnTilemapCellClicked(const FIntPoint& InTileCoords, bool bDoubleClicked) {
}

void FAbstractGraphTilemapDomainMediator::FocusViewportOnTileCoord(const FVector& InCoord) {
    const TSharedPtr<SFlowPreview3DViewport> PreviewViewportPtr = PreviewViewport.Pin();
    if (PreviewViewportPtr.IsValid()) {
        if (ADungeon* PreviewDungeon = PreviewViewportPtr->GetPreviewDungeon()) {
            if (UGridFlowQuery* Query = Cast<UGridFlowQuery>(PreviewDungeon->GetQuery())) {
                if (UGridFlowConfig* Config = Cast<UGridFlowConfig>(PreviewDungeon->GetConfig())) {
                    FVector WorldPosition = Query->ConvertTileToWorldCoord(InCoord);
                    FVector BoxExtent = Config->GridSize * 0.5f;
                    FVector BoxCenter = WorldPosition + BoxExtent.Z;
                    BoxExtent *= 2;
                    const FBox FocusBox(BoxCenter - BoxExtent, BoxCenter + BoxExtent);
                    PreviewViewportPtr->GetViewportClient()->FocusViewportOnBox(FocusBox, false);
                }
            }
        }
    }
}

