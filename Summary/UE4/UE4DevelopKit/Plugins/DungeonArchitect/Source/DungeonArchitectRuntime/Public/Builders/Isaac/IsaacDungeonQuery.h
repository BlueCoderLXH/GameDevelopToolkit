//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Isaac/IsaacDungeonConfig.h"
#include "Builders/Isaac/IsaacDungeonModel.h"
#include "Core/DungeonQuery.h"
#include "IsaacDungeonQuery.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIsaacDungeonQuery, Log, All);

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UIsaacDungeonQuery : public UDungeonQuery {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    FIsaacRoom GetRoom(int32 RoomId);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    void GetFurthestRooms(int32& OutRoomA, int32& OutRoomB);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    bool ContainsDoorBetween(int32 RoomAId, int32 RoomBId);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    FVector GetValidPlatformOnRoom(int32 RoomId);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    FVector GetRandomValidPlatform();

protected:
    virtual void InitializeImpl(UDungeonConfig* InConfig, UDungeonModel* InModel) override;

private:
    UPROPERTY()
    UIsaacDungeonConfig* Config;

    UPROPERTY()
    UIsaacDungeonModel* Model;
};

