//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "FlowEditorSettings.generated.h"

UCLASS()
class UFlowEditorSettings : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Dungeon")
    int32 Seed = 0;

    UPROPERTY(EditAnywhere, Category = "Dungeon")
    bool bRandomizeSeedOnBuild = true;

    UPROPERTY(EditAnywhere, Category = "Dungeon")
    int32 MaxBuildRetries = 50;

    UPROPERTY(EditAnywhere, Category = "Dungeon")
    TMap<FString, FString> ParameterOverrides;
    
    /** The number of timeouts allowed while building the dungeon with multiple retries */
    UPROPERTY(EditAnywhere, Category = "Dungeon")
    int32 NumTimeoutsRetriesAllowed = 20;
};

