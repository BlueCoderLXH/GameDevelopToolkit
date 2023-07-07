//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "DungeonSpatialConstraint.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, BlueprintType, EditInlineNew, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UDungeonSpatialConstraint : public UObject {
    GENERATED_UCLASS_BODY()
public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    bool bApplyBaseRotation;

    /** Should the constraints be rotated to fit the spatial configuration? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    bool bRotateToFitConstraint;

};

