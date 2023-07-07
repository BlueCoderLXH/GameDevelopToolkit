//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DungeonBPFunctionLibrary.generated.h"

class ADungeon;
class ULevelStreamingDynamic;

UCLASS()
class UDungeonBPFunctionLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

    /** Set basic global leap tracking options */
    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    static AActor* SpawnDungeonOwnedActor(ADungeon* Dungeon, TSubclassOf<AActor> ActorClass, const FTransform& Transform);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
	static ULevelStreamingDynamic* StreamDungeonModuleLevel(UObject* WorldContextObject, TSoftObjectPtr<UWorld> Level, int32 InstanceId, const FVector& Location,
			const FRotator& Rotation, bool& bOutSuccess);
	
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    static bool ActorBelongsToDungeon(ADungeon* Dungeon, AActor* ActorToCheck);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    static bool DungeonObjectHasAuthority(UObject* Object);
};

