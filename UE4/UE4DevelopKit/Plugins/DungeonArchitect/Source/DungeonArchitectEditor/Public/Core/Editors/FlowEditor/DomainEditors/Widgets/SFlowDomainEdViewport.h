//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "AssetEditorModeManager.h"
#include "SEditorViewport.h"

DECLARE_DELEGATE_OneParam(FFDViewportActorMouseEvent, AActor*);

// Flow Ed ViewportBase Widget
class DUNGEONARCHITECTEDITOR_API SFlowDomainEdViewport : public SEditorViewport, public FGCObject {
public:
    SLATE_BEGIN_ARGS(SFlowDomainEdViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    ~SFlowDomainEdViewport();

    // FGCObject interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject interface

    /** Set the parent tab of the viewport for determining visibility */
    void SetParentTab(TSharedRef<SDockTab> InParentTab) { ParentTab = InParentTab; }
    
    virtual UWorld* GetWorld() const override;
    TSharedPtr<FPreviewScene> GetPreviewScene() const { return PreviewScene; }

    FFDViewportActorMouseEvent& GetActorSelectionChanged();
    FFDViewportActorMouseEvent& GetActorDoubleClicked();

protected:
    /** SEditorViewport interface */
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual EVisibility OnGetViewportContentVisibility() const override { return EVisibility::Visible; }

private:
    /** Determines the visibility of the viewport. */
    virtual bool IsVisible() const override;

protected:
    /** The parent tab where this viewport resides */
    TWeakPtr<SDockTab> ParentTab;

    /** Level viewport client */
    TSharedPtr<class SFlowDomainEdViewportClient> EditorViewportClient;

    /** The scene for this viewport. */
    TSharedPtr<FPreviewScene> PreviewScene;
};


class DUNGEONARCHITECTEDITOR_API SFlowDomainEdViewportClient
    : public FEditorViewportClient
    , public TSharedFromThis<SFlowDomainEdViewportClient>
{
public:
    SFlowDomainEdViewportClient(FPreviewScene& InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewport);
    FFDViewportActorMouseEvent& GetActorSelectionChanged() { return ActorSelectionChanged; }
    FFDViewportActorMouseEvent& GetActorDoubleClicked() { return ActorDoubleClicked; }
    
    // FEditorViewportClient interface
    virtual void Tick(float DeltaSeconds) override;
    virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
    virtual void SetupViewForRendering(FSceneViewFamily& ViewFamily, FSceneView& View) override;
    // End of FEditorViewportClient interface

private:
    FFDViewportActorMouseEvent ActorSelectionChanged;
    FFDViewportActorMouseEvent ActorDoubleClicked;
};




