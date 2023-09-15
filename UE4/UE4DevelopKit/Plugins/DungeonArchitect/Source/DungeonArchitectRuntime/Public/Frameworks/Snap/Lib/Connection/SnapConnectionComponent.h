//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionConstants.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionInfo.h"

#include "Components/SceneComponent.h"
#include "SnapConnectionComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSnapConnection, Log, All);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class DUNGEONARCHITECTRUNTIME_API USnapConnectionComponent : public USceneComponent {
    GENERATED_UCLASS_BODY()
public:

    virtual void OnRegister() override;


#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

    virtual void Serialize(FArchive& Ar) override;


public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    USnapConnectionInfo* ConnectionInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    ESnapConnectionState ConnectionState = ESnapConnectionState::Door;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    ESnapConnectionDoorType DoorType = ESnapConnectionDoorType::NormalDoor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = SnapMap)
    FString MarkerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    FTransform SpawnOffset = FTransform::Identity;

    UPROPERTY(EditAnywhere, Category = SnapMap)
    ESnapConnectionConstraint ConnectionConstraint;
};

