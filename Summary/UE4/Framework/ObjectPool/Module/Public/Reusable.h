#pragma once
#include "UObject/Interface.h"
#include "Reusable.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogObjectPool, Log, All);

/**
 * UReusable
 * All Object-pooled class should implement this interface so that it can be auto-managed by object pool system
 */
UINTERFACE(BlueprintType)
class OBJECTPOOL_API UReusable : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class OBJECTPOOL_API IReusable
{
	GENERATED_IINTERFACE_BODY()

public:
	/**
	 * Called when object is spawning from Object Pool
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnSpawn();

	/**
	* Called when object is recycling to Object Pool
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnRecycle();
};