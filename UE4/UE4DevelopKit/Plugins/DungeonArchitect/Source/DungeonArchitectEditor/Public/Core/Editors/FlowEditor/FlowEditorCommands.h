//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class DUNGEONARCHITECTEDITOR_API FFlowEditorCommands : public TCommands<FFlowEditorCommands> {
public:
    FFlowEditorCommands();

    virtual void RegisterCommands() override;

public:
    TSharedPtr<FUICommandInfo> Build;
    TSharedPtr<FUICommandInfo> Performance;
    TSharedPtr<FUICommandInfo> ShowEditorSettings;
    TSharedPtr<FUICommandInfo> ShowPreviewDungeonSettings;


};

