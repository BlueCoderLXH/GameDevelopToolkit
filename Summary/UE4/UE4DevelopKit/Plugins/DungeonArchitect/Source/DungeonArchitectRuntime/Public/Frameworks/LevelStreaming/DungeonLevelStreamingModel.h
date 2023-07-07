//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Core/DungeonArchitectConstants.h"
#include "DungeonLevelStreamingModel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLevelStreamingModel, Log, All);

class ADungeon;
class ULevelStreaming;
class ULevelStreamingDynamic;
class UDungeonStreamingChunk;

USTRUCT()
struct FDungeonStreamingChunkParamRep
{
	GENERATED_BODY()

	UPROPERTY()
	FString LevelLongPackageName;

	UPROPERTY()
	FString LogicLevelLongPackageName;

	UPROPERTY()
	uint32 InstanceId;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	int32 OrderNum = -1;

	UPROPERTY()
	FString Category;

	UPROPERTY()
	FGuid ID;

	UPROPERTY()
	FBox Bounds;

	UPROPERTY()
	TArray<FGuid> Neighbors;

	UPROPERTY()
	bool bSpawnRoomChunk = false;
};

UCLASS()
class UDungeonStreamingLevel : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	ULevelStreamingDynamic* LevelStreaming = nullptr;

	UPROPERTY(Transient)
	UPackage* LevelPackage = nullptr;

protected:
	bool bShouldBeVisible = false;

	bool bShouldBeLoaded = false;

	bool bIsLoaded = false;
	
	bool bIsVisible = false;

public:
	void CreateLevelStreaming(UObject* WorldContextObject, const FString& LongPackageName, uint32 InstanceId,
							const FVector& Location, const FRotator& Rotation, bool& bOutSuccess);
	
	void SetStreamingLevelState(bool bInVisible, bool bInLoaded);

	UDungeonStreamingChunk* GetOwner() const;
	
	bool ShouldBeVisible() const { return bShouldBeVisible; }
	bool ShouldBeLoaded() const { return bShouldBeLoaded; }
	bool IsLoaded() const { return bIsLoaded; }
	bool IsVisible() const { return bIsVisible; }
	
	ULevelStreamingDynamic* GetLevelStream() const { return LevelStreaming; }

	ULevel* GetLevel() const { return LevelStreaming ? LevelStreaming->GetLoadedLevel() : nullptr; }

	UPackage* GetLevelPackage() const { return LevelPackage; }

	virtual void OnDestroyChunk();

	bool RequiresStateUpdate(bool bInVisible, bool bInLoaded) const
	{
		return LevelStreaming &&
			((bInVisible != bShouldBeVisible) || (bInLoaded != bShouldBeLoaded));
	}
	
private:
	void RegisterStreamingCallbacks();

protected:
	UFUNCTION()
	virtual void HandleChunkLoaded();

	UFUNCTION()
	virtual void HandleChunkUnloaded();

	UFUNCTION()
	virtual void HandleChunkVisible();

	UFUNCTION()
	virtual void HandleChunkHidden();
};

UCLASS()
class UDungeonLogicStreamingLevel : public UDungeonStreamingLevel
{
	GENERATED_BODY()
	
protected:
	virtual void HandleChunkLoaded() override;

	virtual void HandleChunkUnloaded() override;
	
	virtual void HandleChunkVisible() override;
	
	virtual void HandleChunkHidden() override;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UDungeonStreamingChunk : public UObject
{
	GENERATED_UCLASS_BODY()
	
public:
	UPROPERTY()
	int32 OrderNum = -1;

	UPROPERTY()
	FString Category;

	UPROPERTY()
	FGuid ID;

	UPROPERTY()
	FBox Bounds;

	UPROPERTY()
	TSet<UDungeonStreamingChunk*> Neighbors;

	UPROPERTY()
	bool bSpawnRoomChunk = false;

private:
	// 对应美术制作的关卡
	UPROPERTY(Transient)
	UDungeonStreamingLevel* ArtLevel = nullptr;

	// 对应策划制作的逻辑关卡
	UPROPERTY(Transient)
	UDungeonLogicStreamingLevel* LogicLevel = nullptr;

	TArray<TWeakObjectPtr<AActor>> ManagedActors;

public:
	void CreateLevelStreaming(UObject* WorldContextObject, const FString& LongPackageName, uint32 InstanceId,
	                          const FVector& Location, const FRotator& Rotation, bool& bOutSuccess);
	void CreateLogicLevelStreaming(UObject* WorldContextObject, const FString& LogicLongPackageName, uint32 InstanceId,
	                               const FVector& Location, const FRotator& Rotation, bool& bOutSuccess);

	bool IsRoom() const
	{
		return Category.Contains(DAMarkerConstants::CC_Room);
	}

	bool IsStartRoom() const
	{
		return Category.Contains(DAMarkerConstants::CC_StartRoom);
	}

	bool IsCorridor() const
	{
		return Category.Contains(DAMarkerConstants::CC_Corridor);
	}

	FORCEINLINE_DEBUGGABLE bool ShouldMakeOrder(const FString& OrderCategory) const;
	void MakeOrder(const FString& OrderCategory, int32& CurrentOrder);

	void SetStreamingLevelState(bool bInVisible, bool bInLoaded) const;

	bool RequiresStateUpdate(bool bInVisible, bool bInLoaded) const
	{
		return ArtLevel && ArtLevel->RequiresStateUpdate(bInVisible, bInLoaded);
	}
	
	void RegisterManagedActor(TWeakObjectPtr<AActor> InActor);
	
	virtual void DestroyChunk(UWorld* InWorld);

	UPackage* GetLevelPackage() const;
	UPackage* GetLogicLevelPackage() const;

	ULevel* GetLoadedLevel() const;
	ULevel* GetLoadedLogicLevel() const;

	ULevelStreaming* GetStreamingLevel() const;
	ULevelStreaming* GetLogicStreamingLevel() const;

	FTransform GetLevelTransform() const;

	bool IsLevelLoaded() const
	{
		return ArtLevel && ArtLevel->IsLoaded() &&
			LogicLevel && LogicLevel->IsLoaded();
	}
	
	bool IsLevelVisible() const
	{
		return ArtLevel && ArtLevel->IsVisible() &&
			LogicLevel && LogicLevel->IsVisible();
	}

	UFUNCTION()
	virtual void HandleChunkVisible();

	UFUNCTION()
	virtual void HandleChunkHidden();

	UFUNCTION()
	virtual void HandleChunkLoaded();

	UFUNCTION()
	virtual void HandleChunkUnloaded();

	friend bool operator==(UDungeonStreamingChunk* InChunk, const FGuid& OtherID)
	{
		return InChunk && InChunk->ID == OtherID;
	}
};

class UDungeonLevelStreamingNavigation;
enum class EDungeonLevelStreamChunkSelection : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDALevelStreamerBindableEvent, ADungeon*, Dungeon);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDALevelStreamerStateChangeDelegate, ADungeon*, Dungeon, UDungeonStreamingChunk*, Chunk);

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UDungeonLevelStreamingModel : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY()
	TArray<UDungeonStreamingChunk*> Chunks;

	UPROPERTY()
	UDungeonLevelStreamingNavigation* StreamingNavigation;

	UPROPERTY(BlueprintAssignable, Category = Dungeon)
	FDALevelStreamerBindableEvent OnInitialChunksLoaded;

private:
	bool bNotifiedInitialChunkLoadEvent = false;

	UPROPERTY(ReplicatedUsing=OnRep_ChunkParamsToClient)
	TArray<FDungeonStreamingChunkParamRep> ChunkParamsToClient;	

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Initialize(UWorld* InWorld);
	void Release(UWorld* InWorld);
	bool HasNotifiedInitialChunkLoadEvent() const { return bNotifiedInitialChunkLoadEvent; }
	void NotifyInitialChunksLoaded();
	void GetStreamingSourceLocations(UWorld* InWorld, EDungeonLevelStreamChunkSelection InChunkSelection,
	                                 TArray<FVector>& OutSourceLocations) const;

	void PushChunkParam(const FDungeonStreamingChunkParamRep& InChunkParam)
	{
		ChunkParamsToClient.Add(InChunkParam);
	}

	int32 GetChunkNumberByObject(const UObject* SceneObject) const;

private:
	UFUNCTION()
	void OnRep_ChunkParamsToClient();
};


class DUNGEONARCHITECTRUNTIME_API FLevelStreamLoader
{
public:
	static ULevelStreamingDynamic* LoadLevelInstanceBySoftObjectPtr(UObject* WorldContextObject,
	                                                                const FString& LongPackageName, uint32 InstanceId,
	                                                                FVector Location, FRotator Rotation,
	                                                                bool& bOutSuccess, UPackage*& OutLevelPackage);
private:
	// Counter used by LoadLevelInstance to create unique level names
	static ULevelStreamingDynamic* LoadLevelInstance_Internal(UWorld* World, const FString& LongPackageName,
	                                                          uint32 InstanceId, FVector Location, FRotator Rotation,
	                                                          UPackage*& OutLevelPackage);
};
