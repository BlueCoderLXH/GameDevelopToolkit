#pragma once

#include "CoreMinimal.h"
#include "ObjectPool.generated.h"

DECLARE_DELEGATE_OneParam(FOnSpawnObjectFromPoolDelegate, UObject*);

USTRUCT(BlueprintType)
struct ENGINE_API FObjectPoolConfig
{
	GENERATED_BODY()

	/** Object class type, must implements IReusable */
	UPROPERTY(EditAnywhere, Category=ObjectPoolConfig)
	TSoftClassPtr<UObject> ClassType;

	/** Object pool init capacity */
	UPROPERTY(EditAnywhere, Category=ObjectPoolConfig)
	int32 InitCapacity;

	/** Object pool grow factor (Must greater than 1) */
	UPROPERTY(EditAnywhere, Category=ObjectPoolConfig)
	float GrowFactor;

	/** >0, this is auto reduce time(s) ; <= 0, don't auto reduce */
	UPROPERTY(EditAnywhere, Category=ObjectPoolConfig)
	float AutoReduceTime;

	/** Specify the levels we don't want use ths object pool of this type */
	UPROPERTY(EditAnywhere, Category=ObjectPoolConfig)
	FString ExcludeLevelNames;

	void Reset()
	{
		ClassType = nullptr;
		InitCapacity = 0;
	}
};

USTRUCT()
struct FObjectPoolItemWrapper
{
	GENERATED_BODY()

	UPROPERTY()
	UObject* Obj = nullptr;
	
	UPROPERTY()
	float TotalTimeToBeRemove = 0.f;
	
	UPROPERTY()
	float RemoveTimer = 0.f;
	
	FObjectPoolItemWrapper() { }
	
	FObjectPoolItemWrapper(UObject* InObj, const float InTotalTimeToBeRemove)
		: Obj(InObj)
		, TotalTimeToBeRemove(InTotalTimeToBeRemove)
	{
		RemoveTimer = 0.f;
	}

	FObjectPoolItemWrapper(const FObjectPoolItemWrapper& Other)
	{
		Obj = Other.Obj;
		TotalTimeToBeRemove = Other.TotalTimeToBeRemove;
		RemoveTimer = Other.RemoveTimer;
	}

	bool ShouldAutoRemove() const { return TotalTimeToBeRemove > 0.f; }

	bool Tick(const float DeltaSeconds)
	{
		if (!ShouldAutoRemove())
		{
			return false;
		}
		
		RemoveTimer += DeltaSeconds;
		return RemoveTimer >= TotalTimeToBeRemove;
	}

	void ResetRemoveTimer() { RemoveTimer = 0.f; }

	bool operator==(const FObjectPoolItemWrapper& Other) const
	{
		return Obj == Other.Obj;
	}

	bool operator==(const UObject* InObj) const
	{
		return Obj == InObj;
	}
};

/**
 * UObjectPool
 * ObjectPool with a specified type
 */
UCLASS(BlueprintType, Config = Engine)
class ENGINE_API UObjectPool : public UObject
{
	GENERATED_BODY()

public:
	UObjectPool() {}
	
	bool Init(FObjectPoolConfig& InConfig);
	
	bool InitDefault(const TSoftClassPtr<UObject> ClassType);	

	/**
	 * Spawn a object from pool
	 */
	UObject* Spawn(UObject* InOuter, const FOnSpawnObjectFromPoolDelegate OnSpawnObjectFromPool = FOnSpawnObjectFromPoolDelegate());

	/**
	 * Recycle a object to pool
	 */
	bool Recycle(UObject* RecycleObject);

	/**
	 * Clear the pool
	 */
	void Clear();

	/**
	 * Check curren pool is accessible
	 */
	bool IsAccessible() const
	{
		return bInit && !bPendingRemove;
	}

	/**
	 * Tick ObjectPool' lifetime
	 */
	void Tick(const float DeltaSeconds);

private:
	/**
	 * Check the buffer is empty or not
	 */
	bool Empty() const { return UnusedObjects.Num() <= 0; }

	/**
	 * Expand the buffer
	 */
	bool Expand(const int32 ExpandSize);

	/**
	 * Check whether the buffer is valid or needed to expand
	 */
	bool CheckBufferOnSpawn();
	
	static void RemovePoolItems(TArray<FObjectPoolItemWrapper>& PendingRemoveArray);

private:
	bool bInit = false;
	
	/** Specified object type */
	UPROPERTY()
	FObjectPoolConfig Config;
	
	/** Buffer to cache the usable objects */
	UPROPERTY()
	TArray<FObjectPoolItemWrapper> UnusedObjects;

	/** Buffer to cache the to-be-removed objects */
	UPROPERTY()
	TArray<FObjectPoolItemWrapper> PendingRemoveObjects;
	
	/** Current Capacity of the buffer */
	int32 Capacity;

	/** Is pending-removed */
	bool bPendingRemove;

	UPROPERTY(Config)
	int32 C_DefaultCapacity = 4;
	UPROPERTY(Config)
	int32 C_MinCapacity = 4;
	UPROPERTY(Config)
	float C_MinGrowFactor = 1.2f;
	UPROPERTY(Config)
	int32 C_MinGrowSize = 2;
	UPROPERTY(Config)
	float C_MinAutoReduceTime = 10.f;
	UPROPERTY(Config)
	bool C_ForceGCAfterReduce = false;
};