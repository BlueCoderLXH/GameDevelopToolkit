//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

/**
* Holds the UI commands for the landscape theme editor
*/
class FSnapConnectionEditorCommands
    : public TCommands<FSnapConnectionEditorCommands> {
public:

    /**
    * Default constructor.
    */
    FSnapConnectionEditorCommands();

public:

    // TCommands interface

    virtual void RegisterCommands() override;

public:

    /** Toggles the red channel */
    TSharedPtr<FUICommandInfo> Rebuild;
};

