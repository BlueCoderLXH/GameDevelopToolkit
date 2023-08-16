#pragma once
#include "UObject/Interface.h"
#include "Reusable.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogObjectPool, Log, All);

class UObjectPool;

/**
 * UReusable
 * All Object-pooled class should implement this interface so that it can be auto-managed by object pool system
 */
UINTERFACE(BlueprintType)
class ENGINE_API UReusable : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ENGINE_API IReusable
{
	GENERATED_IINTERFACE_BODY()

public:
	/**
	 * @brief Should the objects of this class use object pool 
	 * @return bool
	 */
	bool ShouldUseObjectPool() const { return bShouldUseObjectPool; }

	/**
	 * @brief Flag this object that is spawned from object pool
	 * @return bool
	 */
	bool IsSpawnedFromPool() const { return bSpawnedFromPool; }


protected:
	/**
	 * @brief [BlueprintNativeEvent] Called when object is spawning from Object Pool
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnSpawn();

	/**
	 * @brief [BlueprintNativeEvent] Called when object is recycling to Object Pool
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnRecycle();

	// Whether should use object pool, if setting it to 'false', the object of this class will be spawned without pool
	bool bShouldUseObjectPool = true;

private:
	/**
	 * @brief Called when object is spawning from Object Pool
	 * @warning Do not call it from outside
	 */
	void SpawnFromPool()
	{
		Execute_OnSpawn(Cast<UObject>(this));
		bSpawnedFromPool = true;
	}

	/**
	 * @brief Called when object is recycling to Object Pool
	 * @warning Do not call it from outside
	 */
	void RecycleToPool()
	{
		Execute_OnRecycle(Cast<UObject>(this));
		bSpawnedFromPool = false;
	}
	
	// Whether this object is spawned from pool
	bool bSpawnedFromPool = false;

	friend UObjectPool;
};