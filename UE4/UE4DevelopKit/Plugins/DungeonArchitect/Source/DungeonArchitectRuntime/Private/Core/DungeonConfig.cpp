//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/DungeonConfig.h"


DEFINE_LOG_CATEGORY(DungeonConfigLog);

UDungeonConfig::UDungeonConfig(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    Seed(0),
    Instanced(false),
    MaxBuildTimePerFrameMs(0) {

}

#if WITH_EDITOR

void UDungeonConfig::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
    //UE_LOG(DungeonConfigLog, Log, TEXT("Config property changed"));
    ConfigPropertyChanged.ExecuteIfBound(e);
}

#endif // WITH_EDITOR

