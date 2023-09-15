//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class DUNGEONARCHITECTEDITOR_API FSnapMapEditorCommands : public TCommands<FSnapMapEditorCommands> {
public:
    FSnapMapEditorCommands();

    virtual void RegisterCommands() override;

public:
    TSharedPtr<FUICommandInfo> BuildGraph;
    TSharedPtr<FUICommandInfo> ValidateGrammarGraph;
    TSharedPtr<FUICommandInfo> Performance;
    TSharedPtr<FUICommandInfo> Settings;
    
    TSharedPtr<FUICommandInfo> DebugStepForward;
    TSharedPtr<FUICommandInfo> DebugRestart;
};

