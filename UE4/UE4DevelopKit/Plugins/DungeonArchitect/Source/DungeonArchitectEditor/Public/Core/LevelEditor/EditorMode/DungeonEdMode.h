//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "EdMode.h"
#include "EditorUndoClient.h"

DECLARE_LOG_CATEGORY_EXTERN(DungeonDrawMode, Log, All);

class ADungeon;
class IDungeonEdTool;
class UDungeonEdModeHandler;

/**
 * DungeonDraw editor mode
 */
class DUNGEONARCHITECTEDITOR_API FEdModeDungeon : public FEdMode, public FEditorUndoClient {
public:

    /** Constructor */
    FEdModeDungeon();

    /** Destructor */
    virtual ~FEdModeDungeon();

    /** FGCObject interface */
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

    /** FEdMode: Called when the mode is entered */
    virtual void Enter() override;

    /** FEdMode: Called when the mode is exited */
    virtual void Exit() override;
	virtual bool UsesToolkits() const override { return true; }

    void RecreateHandler(ADungeon* SelectedDungeon);
    void RecreateToolkit();

    virtual void PostUndo() override {
    }

    // Begin FEditorUndoClient Interface
    virtual void PostUndo(bool bSuccess) override;
    virtual void PostRedo(bool bSuccess) override;
    // End of FEditorUndoClient


    /**
     * Called when the mouse is moved over the viewport
     *
     * @param	InViewportClient	Level editor viewport client that captured the mouse input
     * @param	InViewport			Viewport that captured the mouse input
     * @param	InMouseX			New mouse cursor X coordinate
     * @param	InMouseY			New mouse cursor Y coordinate
     *
     * @return	true if input was handled
     */
    virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;

    /**
     * FEdMode: Called when the mouse is moved while a window input capture is in effect
     *
     * @param	InViewportClient	Level editor viewport client that captured the mouse input
     * @param	InViewport			Viewport that captured the mouse input
     * @param	InMouseX			New mouse cursor X coordinate
     * @param	InMouseY			New mouse cursor Y coordinate
     *
     * @return	true if input was handled
     */
    virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX,
                                   int32 InMouseY) override;

    /** FEdMode: Called when a mouse button is pressed */
    virtual bool StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport) override;

    /** FEdMode: Called when a mouse button is released */
    virtual bool EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport) override;

    /** FEdMode: Called once per frame */
    virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;

    /** FEdMode: Called when a key is pressed */
    virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey,
                          EInputEvent InEvent) override;

    /** FEdMode: Called when mouse drag input it applied */
    virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot,
                            FVector& InScale) override;

    /** FEdMode: Render elements for the DungeonDraw tool */
    virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;

    /** FEdMode: Render HUD elements for this tool */
    virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View,
                         FCanvas* Canvas) override;

    /** FEdMode: Handling SelectActor */
    virtual bool Select(AActor* InActor, bool bInSelected) override;

    /** FEdMode: Check to see if an actor can be selected in this mode - no side effects */
    virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const override;

    /** FEdMode: Called when the currently selected actor has changed */
    virtual void ActorSelectionChangeNotify() override;

    /** Notifies all active modes of mouse click messages. */
    virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy,
                             const FViewportClick& Click) override;

    /** Called when the current level changes */
    void NotifyNewCurrentLevel();

    /** Called when the user changes the current tool in the UI */
    void NotifyToolChanged();

    void OnContentBrowserSelectionChanged(const TArray<FAssetData>& NewSelectedAssets, bool bIsPrimaryBrowser);

    /** FEdMode: widget handling */
    virtual FVector GetWidgetLocation() const override;
    virtual bool AllowWidgetMove() override;
    virtual bool ShouldDrawWidget() const override;
    virtual bool UsesTransformWidget() const override;
    virtual EAxisList::Type GetWidgetAxisToDraw(FWidget::EWidgetMode InWidgetMode) const override;

    virtual bool DisallowMouseDeltaTracking() const override;

    /** Forces real-time perspective viewports */
    void SetRealtimeOverride();
    void RestoreRealtimeOverride();


    /** Apply brush */
    void ApplyBrush(FEditorViewportClient* ViewportClient);

    UDungeonEdModeHandler* GetHandler() const;

    static FEditorModeID EM_Dungeon;


private:
    TSharedPtr<class FDungeonBuildingNotification> DungeonBuildingNotification;
    UDungeonEdModeHandler* ModeHandler;
	static const FText RealtimeDisplayId;
    bool bToolActive;
};

