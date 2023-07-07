//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "DungeonSpawnLogic.generated.h"

class ADungeon;
class UDungeonBuilder;
class UDungeonModel;
class UDungeonConfig;
class UDungeonQuery;

/**
*
*/
UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UDungeonSpawnLogic : public UObject {
    GENERATED_BODY()

public:
    /** Called by the theming engine when a dungeon item is spawned into the scene */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void OnItemSpawn(AActor* Actor, ADungeon* Dungeon);

    virtual void OnItemSpawn_Implementation(AActor* Actor, ADungeon* Dungeon);

};

