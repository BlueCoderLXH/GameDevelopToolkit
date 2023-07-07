//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LevelEditor.h"

class DUNGEONARCHITECTEDITOR_API FEditorUIExtender {
public:
    void Extend();
    void Release();

private:
    TSharedPtr<class FExtender> LevelToolbarExtender;
};


class DUNGEONARCHITECTEDITOR_API FDALevelToolbarCommands : public TCommands<FDALevelToolbarCommands> {
public:
    FDALevelToolbarCommands();
    virtual void RegisterCommands() override;
    void BindCommands();

public:
    TSharedPtr<class FUICommandList> LevelMenuActionList;

    TSharedPtr<FUICommandInfo> OpenLaunchPad;
    TSharedPtr<FUICommandInfo> OpenHelpWindow;
};

