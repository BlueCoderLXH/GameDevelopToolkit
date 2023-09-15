//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Snap/Lib/Connection/Deprecated/SnapConnectionDeprecatedStructs.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"
#include "SnapConnectionInfo.generated.h"

enum class ESnapConnectionInfoVersion {
    InitialVersion = 0,
    ThemeGraphSupport,

    //----------- Versions should be placed above this line -----------------
    LastVersionPlusOne,
    LatestVersion = LastVersionPlusOne - 1
};

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USnapConnectionInfo : public UObject {
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    FName ConnectionCategory;

    /** Check this if you are create a vertical door to move between floors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    bool bVerticalDoor = false;

    UPROPERTY()
    UDungeonThemeAsset* ThemeAsset;

    UPROPERTY()
    int32 Version = 0;

    
    /** Deprecated property. This value is now fetched from the connection theme graph */
    UPROPERTY()
    FSnapConnectionVisualInfo_DEPRECATED WallVisuals;

    /** Deprecated property. This value is now fetched from the connection theme graph */
    UPROPERTY()
    FSnapConnectionVisualInfo_DEPRECATED DoorVisuals;

};

