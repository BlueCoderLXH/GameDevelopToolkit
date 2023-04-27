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
	if (ExpandSize <= 0)
	{
		return false;
	}
	
	const int32 RealExpandSize = FMath::Max(ExpandSize, C_MinGrowSize);
	Capacity += RealExpandSize;

	UnusedObjects.Reserve(Capacity);

	for (int32 i = Capacity; i > (Capacity - RealExpandSize); --i)
	{
		const UClass* ClassType = Config.ClassType.LoadSynchronous();
		if (!ClassType)
		{
			return false;
		}

		const FString RecycleName = FString::Printf(TEXT("%s_Recycled_%d"), *(ClassType->GetName()), i);
		UObject* NewSpawnObject = NewObject<UObject>(GetOuter(), ClassType, FName(RecycleName));	
		if (!NewSpawnObject)
		{
			return false;
		}

		UnusedObjects.Push(NewSpawnObject);

		UE_LOG(LogObjectPool, VeryVerbose, TEXT("UObjectPool::Expand Expand '%s' Success For Type:%s PoolSize:%d Capacity:%d"),
			*RecycleName, *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);
	}
	
	return true;
}

UObject* UObjectPool::Spawn(UObject* InOuter, const FOnSpawnObjectFromPoolDelegate OnSpawnObjectFromPool/* = FOnSpawnObjectFromPoolDelegate()*/)
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

	UObject* SpawnObject = UnusedObjects.Pop(false);
	if (!IsValid(SpawnObject))
	{
		UE_LOG(LogObjectPool, Fatal, TEXT("UObjectPool::Spawn Failed to spawn a object(Type:%s) from object pool!"), *(Config.ClassType.ToString()));
		return nullptr;
	}

	const FString SpawnName = SpawnObject->GetName().Replace(TEXT("Recycled"), TEXT("Spawned"));
	SpawnObject->Rename(*SpawnName, InOuter);

	UE_LOG(LogObjectPool, VeryVerbose, TEXT("UObjectPool::Spawn Spawn '%s' Success For Type:%s PoolSize:%d Capacity:%d"),
		*SpawnName, *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);

	OnSpawnObjectFromPool.ExecuteIfBound(SpawnObject);
	
	IReusable::Execute_OnSpawn(SpawnObject);

	return SpawnObject;
}

bool UObjectPool::Recycle(UObject* RecycleObject)
{
	if (!bInit)
	{
		return false;
	}
	
	if (!Config.ClassType.IsValid())
	{
		UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Recycle ObjectType:%s is invalid!"), *(Config.ClassType.ToString()));
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

	if (UnusedObjects.Contains(RecycleObject))
	{
		return false;
	}

	const FString RecycleName = RecycleObject->GetName().Replace(TEXT("Spawned"), TEXT("Recycled"));	
	RecycleObject->Rename(*RecycleName);
	
	UnusedObjects.Push(RecycleObject);

	UE_LOG(LogObjectPool, VeryVerbose, TEXT("UObjectPool::Recycle Recycle '%s' Success For Type:%s PoolSize:%d Capacity:%d"),
		*RecycleName, *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);

	IReusable::Execute_OnRecycle(RecycleObject);

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
		UE_LOG(LogObjectPool, Verbose, TEXT("UObjectPool::Clear Type:%s PoolSize:%d < PoolCapacity:%d, check it if necessary!"),
			   *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);
	} else {
		UE_LOG(LogObjectPool, Verbose, TEXT("UObjectPool::Clear Type:%s PoolSize:%d PoolCapacity:%d"),
		       *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);
	}
	
	UnusedObjects.Empty();
	Config.Reset();
	Capacity = 0;
}