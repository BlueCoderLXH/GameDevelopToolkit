//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SViewportToolBar.h"

/**
* A level viewport toolbar widget that is placed in a viewport
*/
class DUNGEONARCHITECTEDITOR_API SSnapMapEditorViewportToolBar : public SViewportToolBar {
public:
    SLATE_BEGIN_ARGS(SSnapMapEditorViewportToolBar) {}
        SLATE_ARGUMENT(TSharedPtr<class SSnapMapEditorViewport>, Viewport)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    TSharedRef<SWidget> GeneratePropertiesMenu() const;
    TSharedRef<SWidget> GenerateActionsMenu() const;
    TSharedRef<SWidget> GenerateCameraMenu() const;
    TSharedPtr<FExtender> GetViewMenuExtender();

private:
    /** The viewport that we are in */
    TWeakPtr<class SSnapMapEditorViewport> Viewport;

};


/**
* Class containing commands for level viewport actions
*/
class DUNGEONARCHITECTEDITOR_API
    FSnapMapEditorViewportCommands : public TCommands<FSnapMapEditorViewportCommands> {
public:

    FSnapMapEditorViewportCommands()
        : TCommands<FSnapMapEditorViewportCommands>
        (
            TEXT("SnapMapEditorViewport"), // Context name for fast lookup
            NSLOCTEXT("Contexts", "SnapMapEditorViewport", "Snap Map Editor Viewport"),
            // Localized context name for displaying
            NAME_None, //TEXT("EditorViewport"), // Parent context name.  
            FEditorStyle::GetStyleSetName() // Icon Style Set
        ) {
    }

    //TSharedPtr< FUICommandInfo > ShowPropertyDungeon;

public:
    /** Registers our commands with the binding system */
    virtual void RegisterCommands() override;

};

