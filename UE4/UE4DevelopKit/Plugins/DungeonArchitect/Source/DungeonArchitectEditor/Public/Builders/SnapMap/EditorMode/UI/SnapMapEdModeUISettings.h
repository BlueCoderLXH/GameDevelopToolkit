//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SnapMapEdModeUISettings.generated.h"

UCLASS()
class USnapMapEdModeUISettings : public UObject {
    GENERATED_UCLASS_BODY()

public:

    /** Lets you emit your own markers into the scene */
    UPROPERTY(EditAnywhere, Category = SnapMap)
    bool bNeonVisuals;
};

