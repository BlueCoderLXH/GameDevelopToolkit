//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SnapConnectionConstants.generated.h"

UENUM(BlueprintType)
enum class ESnapConnectionState : uint8 {
    Unknown UMETA(DisplayName = "Unknown"),
    Door UMETA(DisplayName = "Door"),
    Wall UMETA(DisplayName = "Wall"),
};

UENUM(BlueprintType)
enum class ESnapConnectionDoorType : uint8 {
    NotApplicable,
    NormalDoor,
    OneWayDoor,
    OneWayDoorUp,
    OneWayDoorDown,
    LockedDoor,
    CustomDoor,
};

UENUM(BlueprintType)
enum class ESnapConnectionConstraint : uint8 {
    Magnet = 0 UMETA(DisplayName = "Magnet (connect to other Magnets)"),
    PlugMale = 1 UMETA(DisplayName = "Plug Male (connect to other Female Plugs)"),
    PlugFemale = 2 UMETA(DisplayName = "Plug Female (connect to other Male Plugs)")
};

struct DUNGEONARCHITECTRUNTIME_API FSnapConnectionMarkers {
    static const FString Wall;
    static const FString Door;
    static const FString OneWayDoor;
    static const FString OneWayDoorUp;
    static const FString OneWayDoorDown;
};

