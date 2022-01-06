#include "ObjectPool.h"

DEFINE_LOG_CATEGORY(LogObjectPool);

UObjectPool::UObjectPool(const FObjectPoolConfig& InConfig)
{
	Init(InConfig);
}

bool UObjectPool::Init(const FObjectPoolConfig& InConfig)
{
	auto ObjectType = InConfig.ClassType;
	const auto ObjectTypePtr = ObjectType.LoadSynchronous();
	
	checkf(IsValid(ObjectTypePtr), TEXT("UObjectPool::Init ObjectType:%s is invalid!"), *(ObjectType.ToString()));
	checkf(ObjectTypePtr->ImplementsInterface(UReusable::StaticClass()),
		TEXT("UObjectPool::Init ObjectType:%s doesn't implement IReusable!"), *(ObjectType.ToString()));
	
	Config = InConfig;
	Capacity = 0;

	const int32 RealSize = FMath::Max(Config.InitCapacity, C_DefaultCapacity);
	UnusedObjects.Reset(RealSize);
	bInit = Expand(RealSize);

	return bInit;
}

bool UObjectPool::InitDefault(const TSoftClassPtr<UObject> ClassType)
{
	FObjectPoolConfig DefaultConfig;
	DefaultConfig.ClassType = ClassType;
	DefaultConfig.InitCapacity = C_DefaultCapacity;
	DefaultConfig.bAutoReduce = C_AutoReduce;

	return Init(DefaultConfig);
}

bool UObjectPool::Expand(const int32 ExpandSize)
{
	const int32 RealExpandSize = FMath::Max(ExpandSize, C_MinGrowSize);

	for (int32 i = 0; i < RealExpandSize; i++)
	{
		UClass* ClassType = Config.ClassType.LoadSynchronous();
		if (!ClassType)
		{
			return false;
		}

		const FString RecycleName = FString::Printf(TEXT("%s_Recycled_%d"), *(ClassType->GetName()), (Capacity + i));
		UObject* NewSpawnObject = NewObject<UObject>(GetOuter(), ClassType, FName(RecycleName));	
		if (!NewSpawnObject)
		{
			return false;
		}

		IReusable::Execute_OnRecycle(NewSpawnObject);
		
		UnusedObjects.Add(NewSpawnObject);
	}

	Capacity += RealExpandSize;

	return true;
}

UObject* UObjectPool::Spawn(UObject* InOuter)
{
	if (!bInit)
	{
		return nullptr;
	}
	
	if (!Config.ClassType.IsValid())
	{
		UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Spawn ObjectType:%s is invalid!"), *(Config.ClassType.ToString()));
		return nullptr;
	}
	
	if (Empty())
	{
		const int32 ExpandCount = Capacity * (C_GrowFactor - 1);

		if (!Expand(ExpandCount))
		{
			UE_LOG(LogObjectPool, Fatal, TEXT("UObjectPool::Spawn Failed to expand pool for Type:%s !"), *(Config.ClassType.ToString()));
			return nullptr;
		}
	}

	UObject* SpawnObject = UnusedObjects[0];
	if (!SpawnObject)
	{
		UE_LOG(LogObjectPool, Fatal, TEXT("UObjectPool::Spawn Failed to spawn a object(Type:%s) from object pool!"), *(Config.ClassType.ToString()));
		return nullptr;
	}
	
	UnusedObjects.RemoveAt(0);

	const FString SpawnName = SpawnObject->GetName().Replace(TEXT("Recycled"), TEXT("Spawned"));
	SpawnObject->Rename(*SpawnName, InOuter);

	UE_LOG(LogObjectPool, Verbose, TEXT("UObjectPool::Spawn Spawn '%s' Success For Type:%s PoolSize:%d Capacity:%d"),
		*SpawnName, *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);

	// IReusable::Execute_OnSpawn(SpawnObject);

	return SpawnObject;
}

bool UObjectPool::Recycle(UObject* RecycleObject)
{
	if (!bInit)
	{
		return false;
	}
	
	if (Config.ClassType.IsNull())
	{
		UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Recycle ObjectType is invalid!"), *(Config.ClassType.ToString()));
		return false;
	}
	
	if (!IsValid(RecycleObject))
	{
		UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Recycle 'RecycleObject' is invalid!"));
		return false;
	}

	if (RecycleObject->GetClass() != Config.ClassType.Get())
	{
		UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Recycle RecycleObject's type doesn't match the object pool's!"));
		return false;
	}

	const FString RecycleName = RecycleObject->GetName().Replace(TEXT("Spawned"), TEXT("Recycled"));	
	RecycleObject->Rename(*RecycleName);
	
	UnusedObjects.Push(RecycleObject);

	UE_LOG(LogObjectPool, Verbose, TEXT("UObjectPool::Recycle Recycle '%s' Success For Type:%s PoolSize:%d Capacity:%d"),
		*RecycleName, *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);

	// IReusable::Execute_OnRecycle(RecycleObject);

	return true;
}

void UObjectPool::Clear()
{
	if (!bInit)
	{
		return;
	}
	
	if (Capacity != UnusedObjects.Num())
	{
		UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Clear not all object has been recycled to this object pool!"));
	}
	
	UE_LOG(LogObjectPool, VeryVerbose, TEXT("UObjectPool::Clear Type:%s PoolSize:%d Capacity:%d"),
		*(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);
	
	UnusedObjects.Empty();
	Config.Reset();
	Capacity = 0;
}