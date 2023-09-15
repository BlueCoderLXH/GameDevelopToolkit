//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEditor.h"

#include "UObject/GCObject.h"

class SGraphEditor;
class UGridFlowTilemap;
class UGridFlowTilemapEdGraph;
class UFlowGraphItem;

class FFlowDomainEdTilemap
    : public IFlowDomainEditor
    , public FGCObject
{
public:
    //~ Begin FFlowDomainEditorBase Interface
    virtual FFlowDomainEditorTabInfo GetTabInfo() const override;
    virtual TSharedRef<SWidget> GetContentWidget() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Build(FFlowExecNodeStatePtr State) override;
    virtual bool CanSaveThumbnail() const override { return true; }
    virtual void SaveThumbnail(const struct FAssetData& InAsset, int32 ThumbSize) override;
    //~ End FFlowDomainEditorBase Interface

    //~ Begin FGCObject Interface
    virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;
    //~ End FGCObject Interface

    void SelectItem(const FGuid& InItemId) const;
    void SelectChunk(const FVector& InChunkCoord, bool bInSelected);
    void RefreshPreviewTilemap();
    
    class IMediator {
    public:
        virtual ~IMediator() {}
        virtual void OnTilemapItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) = 0;
        virtual void OnTilemapCellClicked(const FIntPoint& InTileCoords, bool bDoubleClicked) = 0;
        virtual void UpdateTilemapChunkSelection(const FVector& InChunkCoords) = 0;
        virtual void GetAllTilemapItems(FFlowExecNodeStatePtr State, TArray<UFlowGraphItem*>& OutItems) = 0;
        virtual void FocusViewportOnTileCoord(const FVector& InCoord) = 0;
    };
    FORCEINLINE void SetMediator(TSharedPtr<IMediator> InMediator) { Mediator = InMediator; }

private:
    virtual void InitializeImpl(TSharedPtr<IDetailsView> PropertyEditor) override;
    virtual IFlowDomainPtr CreateDomain() const override;
    
    void OnPreviewTilemapCellClicked(const FIntPoint& InTileCoords, bool bDoubleClicked);
    void FocusOnTileCoord(const FIntPoint& InTileCoords);
    FORCEINLINE bool IsTileCellSelected(const FVector& InChunkCoord) const;
    void OnItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked);

private:
    TSharedPtr<SGraphEditor> TilemapGraphEditor;
    TSharedPtr<class FGridFlowTilemapGraphHandler> TilemapGraphHandler;
    UGridFlowTilemapEdGraph* TilemapGraph = nullptr;

    TWeakPtr<IMediator> Mediator;
    TWeakPtr<FFlowExecNodeState> PreviewState;
    
    FVector SelectedChunkCoord;
    bool bChunkSelected = false;
};

