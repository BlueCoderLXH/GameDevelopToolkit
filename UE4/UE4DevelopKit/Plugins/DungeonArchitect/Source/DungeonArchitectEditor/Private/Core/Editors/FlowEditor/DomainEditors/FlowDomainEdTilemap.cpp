//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdTilemap.h"

#include "Builders/GridFlow/GridFlowQuery.h"
#include "Core/Editors/FlowEditor/FlowEditorUtils.h"
#include "Frameworks/Flow/Domains/Tilemap/Graph/TilemapGraphInfrastructure.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapDomain.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapRenderer.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"

#include "Engine/TextureRenderTarget2D.h"
#include "IDetailsView.h"

#define LOCTEXT_NAMESPACE "FlowDomainEdAbstractGraph"
DEFINE_LOG_CATEGORY_STATIC(LogDomainEdTilemap, Log, All);

void FFlowDomainEdTilemap::InitializeImpl(TSharedPtr<IDetailsView> PropertyEditor) {
    TilemapGraph = NewObject<UGridFlowTilemapEdGraph>();
    TilemapGraph->Initialize();
    TilemapGraph->OnCellClicked.BindRaw(this, &FFlowDomainEdTilemap::OnPreviewTilemapCellClicked);
    TilemapGraph->OnItemWidgetClicked.BindRaw(this, &FFlowDomainEdTilemap::OnItemWidgetClicked);

    // Create the appearance info
    FGraphAppearanceInfo AppearanceInfo;
    AppearanceInfo.CornerText = LOCTEXT("GridFlowTilemapGraphBranding", "Tilemap");
    TilemapGraphHandler = MakeShareable(new FGridFlowTilemapGraphHandler);
    TilemapGraphHandler->Bind();
    TilemapGraphHandler->SetPropertyEditor(PropertyEditor);

    TilemapGraphEditor = SNew(SGraphEditor)
        .AdditionalCommands(TilemapGraphHandler->GraphEditorCommands)
        .Appearance(AppearanceInfo)
        .GraphToEdit(TilemapGraph)
        .IsEditable(true)
        .ShowGraphStateOverlay(false)
        .GraphEvents(TilemapGraphHandler->GraphEvents);

    TilemapGraphHandler->SetGraphEditor(TilemapGraphEditor);
}

void FFlowDomainEdTilemap::OnPreviewTilemapCellClicked(const FIntPoint& InTileCoords, bool bDoubleClicked) {
    bool bRequestSelection = false;
    FVector RequestedChunkCoord;

    TSharedPtr<FFlowExecNodeState> State = PreviewState.Pin();
    UGridFlowTilemap* Tilemap = State.IsValid() ? State->GetState<UGridFlowTilemap>(UGridFlowTilemap::StateTypeID) : nullptr;
    TSharedPtr<IMediator> MediatorPtr = Mediator.Pin();
    
    if (Tilemap) {
        const FGridFlowTilemapCell* CellPtr = Tilemap->GetSafe(InTileCoords.X, InTileCoords.Y);
        if (CellPtr) {
            const FGridFlowTilemapCell& Cell = *CellPtr;
            if (Cell.bLayoutCell) {
                RequestedChunkCoord = Cell.ChunkCoord;
                bRequestSelection = true;
            }
            else {
                bRequestSelection = false;
            }
        }

        bool bRequiresUpdate = true;

        if (!bChunkSelected && !bRequestSelection) {
            bRequiresUpdate = false;
        }

        if (bChunkSelected == bRequestSelection && SelectedChunkCoord.Equals(RequestedChunkCoord)) {
            bRequiresUpdate = false;
        }

        if (bRequiresUpdate) {
            if (MediatorPtr.IsValid()) {
                MediatorPtr->UpdateTilemapChunkSelection(RequestedChunkCoord);
            }
            
        }
    }

    SelectItem(FGuid());

    if (MediatorPtr.IsValid()) {
        MediatorPtr->OnTilemapCellClicked(InTileCoords, bDoubleClicked);
    }

    if (bDoubleClicked) {
        FocusOnTileCoord(InTileCoords);
    }
}

void FFlowDomainEdTilemap::SaveThumbnail(const struct FAssetData& InAsset, int32 ThumbSize) {
    
    UTexture* TilemapTexture = TilemapGraph ? TilemapGraph->GetPreviewTexture() : nullptr;
    if (!TilemapTexture) return;

    UTextureRenderTarget2D* RTT = Cast<UTextureRenderTarget2D>(TilemapTexture);
    FFlowEditorUtils::SaveTextureAsAssetThumbnail(InAsset, ThumbSize, RTT);
}

void FFlowDomainEdTilemap::FocusOnTileCoord(const FIntPoint& InTileCoords) {
    TSharedPtr<FFlowExecNodeState> State = PreviewState.Pin();
    UGridFlowTilemap* Tilemap = State.IsValid() ? State->GetState<UGridFlowTilemap>(UGridFlowTilemap::StateTypeID) : nullptr;
    
    if (Tilemap) {
        float HeightCoord = 0;
        const FGridFlowTilemapCell* CellPtr = Tilemap->GetSafe(InTileCoords.X, InTileCoords.Y);
        if (CellPtr) {
            HeightCoord = CellPtr->Height;
            const FVector Coord(InTileCoords.X, InTileCoords.Y, HeightCoord);
            TSharedPtr<IMediator> MediatorPtr = Mediator.Pin();
            if (MediatorPtr.IsValid()) {
                MediatorPtr->FocusViewportOnTileCoord(Coord);
            }
        }
    }
}

void FFlowDomainEdTilemap::Build(FFlowExecNodeStatePtr State) {
    PreviewState = State;
    UGridFlowTilemap* Tilemap = State->GetState<UGridFlowTilemap>(UGridFlowTilemap::StateTypeID);
    if (TilemapGraph) {
        TArray<UFlowGraphItem*> Items;
        TSharedPtr<IMediator> MediatorPtr = Mediator.Pin();
        if (MediatorPtr.IsValid()) {
            MediatorPtr->GetAllTilemapItems(State, Items);
        }

        FGridFlowTilemapRendererSettings RenderSettings;
        RenderSettings.bUseTextureTileSize = false;
        RenderSettings.TileSize = 10;
        RenderSettings.BackgroundColor = FLinearColor::Black;
        RenderSettings.FuncCellSelected =
            [this](const FVector& InChunkCoord) -> bool {
                return IsTileCellSelected(InChunkCoord);
        };

        TilemapGraph->GeneratePreviewTexture(Tilemap, RenderSettings, Items);
    }
}

IFlowDomainPtr FFlowDomainEdTilemap::CreateDomain() const {
    return MakeShareable(new FFlowTilemapDomain);
}

bool FFlowDomainEdTilemap::IsTileCellSelected(const FVector& InChunkCoord) const {
    return bChunkSelected ? InChunkCoord.Equals(SelectedChunkCoord) : false;
}

void FFlowDomainEdTilemap::SelectItem(const FGuid& InItemId) const {
    TilemapGraph->SelectedItemId = InItemId;
}

void FFlowDomainEdTilemap::SelectChunk(const FVector& InChunkCoord, bool bInSelected) {
    SelectedChunkCoord = InChunkCoord;
    bChunkSelected = bInSelected;
}

void FFlowDomainEdTilemap::RefreshPreviewTilemap() {
    const FFlowExecNodeStatePtr State = PreviewState.Pin();
    if (State.IsValid()) {
        Build(State);
    }
}

void FFlowDomainEdTilemap::OnItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) {
    SelectItem(InItemId);

    TSharedPtr<IMediator> MediatorPtr = Mediator.Pin();
    if (MediatorPtr.IsValid()) {
        MediatorPtr->OnTilemapItemWidgetClicked(InItemId, bDoubleClicked);
    }
}

FFlowDomainEditorTabInfo FFlowDomainEdTilemap::GetTabInfo() const {
    static const FFlowDomainEditorTabInfo TabInfo = {
        TEXT("TilemapTab"),
        LOCTEXT("TilemapTabLabel", "Tilemap"),
        FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details")
    };

    return TabInfo;
}

TSharedRef<SWidget> FFlowDomainEdTilemap::GetContentWidget() {
    return TilemapGraphEditor.ToSharedRef();
}

void FFlowDomainEdTilemap::AddReferencedObjects(FReferenceCollector& Collector) {
    if (TilemapGraph) {
        Collector.AddReferencedObject(TilemapGraph);
    }
}

void FFlowDomainEdTilemap::Tick(float DeltaTime) {
}

#undef LOCTEXT_NAMESPACE

