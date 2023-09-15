//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "PreviewScene.h"
#include "SEditorViewport.h"

class ADungeon;
class UFlowAssetBase;
class FFlowEditorBase;
class ASkyLight;

class DUNGEONARCHITECTEDITOR_API SFlowPreview3DViewport : public SEditorViewport, public FGCObject {
public:
    SLATE_BEGIN_ARGS(SFlowPreview3DViewport) {}
        SLATE_ARGUMENT(TWeakObjectPtr<UFlowAssetBase>, FlowAsset)
        SLATE_ARGUMENT(TWeakPtr<FFlowEditorBase>, FlowEditor)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    ~SFlowPreview3DViewport();

    // FGCObject interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject interface

    /** Set the parent tab of the viewport for determining visibility */
    void SetParentTab(TSharedRef<SDockTab> InParentTab) { ParentTab = InParentTab; }

    // SWidget Interface
    virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
    // End of SWidget Interface

    EVisibility GetToolbarVisibility() const;


    virtual UWorld* GetWorld() const override;
    ADungeon* GetPreviewDungeon() const { return PreviewDungeon.Get(); }
    void SetPreviewDungeon(ADungeon* InDungeon);

protected:
    /** SEditorViewport interface */
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual EVisibility OnGetViewportContentVisibility() const override;
    virtual void BindCommands() override;

    virtual void OnFocusViewportToSelection() override;
    virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

    void OnToggleDebugData();
    void OnDisplayDungeonProperties();

private:
    /** Determines the visibility of the viewport. */
    virtual bool IsVisible() const override;

private:
    /** The parent tab where this viewport resides */
    TWeakPtr<SDockTab> ParentTab;

    /** Level viewport client */
    TSharedPtr<class FFlowPreview3DViewportClient> EditorViewportClient;

    /** The scene for this viewport. */
    TSharedPtr<FPreviewScene> PreviewScene;
    
    TWeakObjectPtr<ADungeon> PreviewDungeon;

    TWeakObjectPtr<UFlowAssetBase> FlowAsset;
    TWeakPtr<FFlowEditorBase> FlowEditor;
};

