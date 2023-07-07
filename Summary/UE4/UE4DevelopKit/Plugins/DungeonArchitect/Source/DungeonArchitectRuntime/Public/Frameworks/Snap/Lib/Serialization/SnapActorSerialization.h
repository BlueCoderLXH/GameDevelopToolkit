//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/Interface.h"
#include "SnapActorSerialization.generated.h"

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSnapChunkActorDataEntry {
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    TSubclassOf<AActor> ActorClass;

    UPROPERTY()
    FTransform ActorTransform;

    UPROPERTY()
    FString ActorName;

    UPROPERTY()
    TArray<uint8> ActorData;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API USnapStreamingChunkActorData : public UObject {
    GENERATED_BODY()
public:

    UPROPERTY()
    TArray<FSnapChunkActorDataEntry> ActorEntries;

public:
    void SaveLevel(ULevel* InLevel);
    void LoadLevel(ULevel* InLevel);

private:
    void SaveActor(AActor* InActor, FSnapChunkActorDataEntry& OutEntry);
    void LoadActor(AActor* InActor, const FSnapChunkActorDataEntry& InEntry);

};

UINTERFACE(BlueprintType)
class DUNGEONARCHITECTRUNTIME_API USnapSerializable : public UInterface {
    GENERATED_BODY()

};

class DUNGEONARCHITECTRUNTIME_API ISnapSerializable {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dungeon")
    void OnSnapDataLoaded();

};

