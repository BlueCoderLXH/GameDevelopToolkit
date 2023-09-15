//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class FMenuBuilder;
class FExtender;
class FUICommandList;

class FDAHelpMenuExtender {
public:
    void Extend();
    void Release();

private:
    static void AddDocMenuCommands(FMenuBuilder& MenuBuilder);
    static void CreateDocSubMenu(FMenuBuilder& MenuBuilder);

private:
    TSharedPtr<FUICommandList> GlobalLevelEditorActions;
    TSharedPtr<FExtender> MenuExtender;
};

