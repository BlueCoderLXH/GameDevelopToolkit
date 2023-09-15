//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/SnapConnectionEditor/SnapConnectionEditor.h"

#include "SEditorViewport.h"

class USnapConnectionInfo;
class ASnapConnectionActor;
class USnapConnectionComponent;
class FAdvancedPreviewScene;

/**
* StaticMesh Editor Preview viewport widget
*/
class DUNGEONARCHITECTEDITOR_API SSnapConnectionPreview3DViewport : public SEditorViewport, public FGCObject {
public:
    SLATE_BEGIN_ARGS(SSnapConnectionPreview3DViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    ~SSnapConnectionPreview3DViewport();

    // FGCObject interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject interface

    /** Set the parent tab of the viewport for determining visibility */
    void SetParentTab(TSharedRef<SDockTab> InParentTab) { ParentTab = InParentTab; }

    // SWidget Interface
    virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
    // End of SWidget Interface

    EVisibility GetToolbarVisibility() const;

    void SpawnPreviewActor();

    USnapConnectionComponent* GetConnectionComponent() const;
    ASnapConnectionActor* GetConnectionActor() const;

    TSharedPtr<class FAdvancedPreviewScene> GetAdvancedPreview() const { return PreviewScene; }
    
    virtual UWorld* GetWorld() const override;
protected:
    /** SEditorViewport interface */
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual EVisibility OnGetViewportContentVisibility() const override;
    virtual void BindCommands() override;
    virtual void OnFocusViewportToSelection() override;
    virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

    void OnToggleDebugData();

private:
    /** Determines the visibility of the viewport. */
    virtual bool IsVisible() const override;


private:
    /** The parent tab where this viewport resides */
    TWeakPtr<SDockTab> ParentTab;

    /** Level viewport client */
    TSharedPtr<class FSnapConnectionPreview3DViewportClient> EditorViewportClient;

    /** The scene for this viewport. */
    TSharedPtr<FAdvancedPreviewScene> PreviewScene;
    ASnapConnectionActor* PreviewActor = nullptr;
    USkyAtmosphereComponent* AtmosphericFog = nullptr;

};

