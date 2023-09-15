//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FUICommandInfo;

class FDungeonArchitectHelpSystemCommands : public TCommands<FDungeonArchitectHelpSystemCommands> {
public:
    FDungeonArchitectHelpSystemCommands();

    virtual void RegisterCommands() override;

public:
    TSharedPtr<FUICommandInfo> OpenDocumentationUserGuide;
    TSharedPtr<FUICommandInfo> OpenDocumentationQuickStartGuide;
    TSharedPtr<FUICommandInfo> OpenDocumentationVideoTutorials;

};

