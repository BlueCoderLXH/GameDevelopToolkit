// #pragma once
//
// #include "Reusable.h"
// #include "CoreMinimal.h"
// #include "ObjectPool.generated.h"
//
// DECLARE_DELEGATE_OneParam(FOnSpawnObjectFromPoolDelegate, UObject*);
//
// USTRUCT(BlueprintType)
// struct OBJECTPOOL_API FObjectPoolConfig
// {
// 	GENERATED_BODY()
//
// 	/** Object class type, must implements IReusable */
// 	UPROPERTY(EditAnywhere, Category=ObjectPoolConfig)
// 	TSoftClassPtr<UObject> ClassType;
//
// 	/** Object pool init capacity */
// 	UPROPERTY(EditAnywhere, Category=ObjectPoolConfig)
// 	int32 InitCapacity;
//
// 	/** Object pool reduce capacity automatically */
// 	UPROPERTY(EditAnywhere, Category=ObjectPoolConfig)
// 	bool bAutoReduce;
//
// 	/** Specify the levels we don't want use ths object pool of this type */
// 	UPROPERTY(EditAnywhere, Category=ObjectPoolConfig)
// 	FString ExcludeLevelNames;
//
// 	void Reset()
// 	{
// 		ClassType = nullptr;
// 		InitCapacity = 0;
// 	}
// };
//
// /**
//  * UObjectPool
//  * ObjectPool with a specified type
//  */
// UCLASS(BlueprintType, Config = Engine)
// class OBJECTPOOL_API UObjectPool : public UObject
// {
// 	GENERATED_BODY()
//
// public:
// 	UObjectPool() {}
// 	
// 	UObjectPool(const FObjectPoolConfig& InConfig);
// 	
// 	bool Init(UPARAM(ref) const FObjectPoolConfig& InConfig);
// 	
// 	bool InitDefault(const TSoftClassPtr<UObject> ClassType);	
//
// 	/**
// 	 * Spawn a object from pool
// 	 */
// 	UObject* Spawn(UObject* InOuter, const FOnSpawnObjectFromPoolDelegate OnSpawnObjectFromPool = FOnSpawnObjectFromPoolDelegate());
//
// 	/**
// 	 * Recycle a object to pool
// 	 */
// 	bool Recycle(UObject* RecycleObject);
//
// 	/**
// 	 * Clear the pool
// 	 */
// 	void Clear();
//
// private:
// 	/**
// 	 * Check the buffer is empty or not
// 	 */
// 	bool Empty() const { return UnusedObjects.Num() <= 0; }
//
// 	/**
// 	 * Expand the buffer
// 	 */
// 	bool Expand(const int32 ExpandSize);
//
// private:
// 	bool bInit = false;
// 	
// 	/** Specified object type */
// 	UPROPERTY()
// 	FObjectPoolConfig Config;
// 	
// 	/** Buffer to cache the usable objects */
// 	UPROPERTY()
// 	TArray<UObject*> UnusedObjects;
// 	
// 	/** Current Capacity of the buffer */
// 	int32 Capacity;
//
// 	UPROPERTY(Config)
// 	int32 C_DefaultCapacity = 16;
// 	UPROPERTY(Config)
// 	int32 C_MinCapacity = 4;
// 	UPROPERTY(Config)
// 	float C_GrowFactor = 1.4;
// 	UPROPERTY(Config)
// 	int32 C_MinGrowSize = 4;
// 	UPROPERTY(Config)
// 	bool C_AutoReduce;
// };