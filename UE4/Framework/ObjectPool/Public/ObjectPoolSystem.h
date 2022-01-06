#pragma once

#include "ObjectPool.h"
#include "ObjectPoolSystem.generated.h"

/**
 * UObjectPoolSystem
 *
 * Object pool system with specify-typed object pool, you can add object pool by two ways:
 * (1) configuring in BaseEngine.ini or DefaultEngine.ini file
 * (2) call blueprinted function "Add" manually
 */
UCLASS(BlueprintType, Config = Engine)
class OBJECTPOOL_API UObjectPoolSystem : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Init object pool system
	 */
	UFUNCTION(BlueprintCallable, Category=ObjectPoolSystem)
	void Init();

	/**
	 * Add a object pool with config
	 */
	UFUNCTION(BlueprintCallable, Category=ObjectPoolSystem)
	void Add(UPARAM(ref) const FObjectPoolConfig& PoolConfig);

	/**
	* Add a object pool with default config
	*/
	UFUNCTION(BlueprintCallable, Category=ObjectPoolSystem)
	void AddDefault(const TSoftClassPtr<UObject> ClassType);

	/**
	* Remove a non-used object pool with specified type
	*/
	UFUNCTION(BlueprintCallable, Category=ObjectPoolSystem)
	void Remove(UPARAM(ref) const TSoftClassPtr<UObject>& ClassType);

	/**
	 * Spawn a object from pool
	 */
	UFUNCTION(BlueprintCallable, Category=ObjectPoolSystem)
	UObject* Spawn(UPARAM(ref) const TSoftClassPtr<UObject>& ClassType, UObject* InOuter);

	/**
	 * Recycle a object to pool
	 */
	UFUNCTION(BlueprintCallable, Category=ObjectPoolSystem)
	bool Recycle(UObject* RecycleObject);

	/**
	 * Clear the pool cache, usually called when current world is being destroyed
	 */
	UFUNCTION(BlueprintCallable, Category=ObjectPoolSystem)
	void Clear();

private:
	static bool CommonCheck(const TSoftClassPtr<UObject> ClassType);
	
	static bool CanLevelUseObjectPool(const FString& LevelName, const FString& ExcludeLevelNames);
	
	void AddInner(const FObjectPoolConfig& PoolConfig, const bool bMakeDefault = false);
	
	/** Config data from .ini file */
	UPROPERTY(Config)
	TArray<FObjectPoolConfig> ConfigArray;

	/** Pool cache map */
	UPROPERTY()
	TMap<TSoftClassPtr<UObject>, UObjectPool*> PoolMap;

	/** Whether init or not */
	bool bInit = false;
};