//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SViewportToolBar.h"

/**
* A level viewport toolbar widget that is placed in a viewport
*/
class DUNGEONARCHITECTEDITOR_API SDungeonEditorViewportToolBar : public SViewportToolBar {
public:
    SLATE_BEGIN_ARGS(SDungeonEditorViewportToolBar) {}
        SLATE_ARGUMENT(TSharedPtr<class SDungeonEditorViewport>, Viewport)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    TSharedRef<SWidget> GeneratePropertiesMenu() const;
    TSharedRef<SWidget> GenerateActionsMenu() const;
    TSharedRef<SWidget> GenerateCameraMenu() const;
    TSharedPtr<FExtender> GetViewMenuExtender();

private:
    /** The viewport that we are in */
    TWeakPtr<class SDungeonEditorViewport> Viewport;

};


/**
* Class containing commands for level viewport actions
*/
class DUNGEONARCHITECTEDITOR_API FDungeonEditorViewportCommands : public TCommands<FDungeonEditorViewportCommands> {
public:

    FDungeonEditorViewportCommands()
        : TCommands<FDungeonEditorViewportCommands>
        (
            TEXT("DungeonEditorViewport"), // Context name for fast lookup
            NSLOCTEXT("Contexts", "DungeonViewport", "Dungeon Viewport"), // Localized context name for displaying
            NAME_None, //TEXT("EditorViewport"), // Parent context name.  
            FEditorStyle::GetStyleSetName() // Icon Style Set
        ) {
    }

    TSharedPtr<FUICommandInfo> ShowPropertyDungeon;
    TSharedPtr<FUICommandInfo> ShowPropertySkylight;
    TSharedPtr<FUICommandInfo> ShowPropertyDirectionalLight;
    TSharedPtr<FUICommandInfo> ShowPropertyAtmosphericFog;
    TSharedPtr<FUICommandInfo> ToggleDebugData;

    TSharedPtr<FUICommandInfo> ForceRebuildPreviewLayout;
    TSharedPtr<FUICommandInfo> DrawDebugData;
public:
    /** Registers our commands with the binding system */
    virtual void RegisterCommands() override;

};

