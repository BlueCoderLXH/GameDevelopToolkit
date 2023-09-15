//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "WorkflowOrientedApp/ApplicationMode.h"

class FSnapMapEdAppModeBase
    : public FApplicationMode {
public:
    FSnapMapEdAppModeBase(FName InModeName);
    virtual void Tick(float DeltaTime);
    virtual void OnAssetSave();
};

