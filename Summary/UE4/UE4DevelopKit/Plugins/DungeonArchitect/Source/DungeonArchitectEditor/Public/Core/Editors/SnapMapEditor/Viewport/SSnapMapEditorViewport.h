//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SEditorViewport.h"

class ADungeon;
class USnapMapAsset;
class FSnapMapEditor;

class DUNGEONARCHITECTEDITOR_API SSnapMapEditorViewport : public SEditorViewport, public FGCObject {
public:
    SLATE_BEGIN_ARGS(SSnapMapEditorViewport) {}
        SLATE_ARGUMENT(TWeakPtr<FSnapMapEditor>, DungeonFlowEditor)
        SLATE_ARGUMENT(USnapMapAsset*, ObjectToEdit)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    ~SSnapMapEditorViewport();

    // FGCObject interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject interface

    /** Set the parent tab of the viewport for determining visibility */
    void SetParentTab(TSharedRef<SDockTab> InParentTab) { ParentTab = InParentTab; }

    // SWidget Interface
    virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
    // End of SWidget Interface

    EVisibility GetToolbarVisibility() const;
    TSharedPtr<class FAdvancedPreviewScene> GetAdvancedPreview() const { return PreviewScene; }

protected:
    /** SEditorViewport interface */
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual EVisibility OnGetViewportContentVisibility() const override;
    virtual void BindCommands() override;
    virtual void OnFocusViewportToSelection() override;
    virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

private:
    /** Determines the visibility of the viewport. */
    virtual bool IsVisible() const override;

private:
    TWeakPtr<FSnapMapEditor> DungeonFlowEditorPtr;
    USnapMapAsset* ObjectToEdit;

    /** The parent tab where this viewport resides */
    TWeakPtr<SDockTab> ParentTab;

    /** Level viewport client */
    TSharedPtr<class FSnapMapEditorViewportClient> EditorViewportClient;

    /** The scene for this viewport. */
    TSharedPtr<class FAdvancedPreviewScene> PreviewScene;
};

