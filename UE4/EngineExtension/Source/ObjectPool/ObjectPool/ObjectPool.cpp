#include "ObjectPool.h"
#include "Reusable.h"

DEFINE_LOG_CATEGORY(LogObjectPool);

bool UObjectPool::Init(FObjectPoolConfig& InConfig, UObject* InObjectPoolRoot)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UOBJECTPOOL_INIT);

	if (InObjectPoolRoot)
	{
		ObjectPoolRoot = InObjectPoolRoot;
	}
	
	const auto ObjectType = InConfig.ClassType;
	const auto ObjectTypePtr = ObjectType.LoadSynchronous();
	
	checkf(IsValid(ObjectTypePtr), TEXT("UObjectPool::Init ObjectType:%s is invalid!"), *(ObjectType.ToString()));
	checkf(ObjectTypePtr->ImplementsInterface(UReusable::StaticClass()),
		TEXT("UObjectPool::Init ObjectType:%s doesn't implement IReusable!"), *(ObjectType.ToString()));

	InConfig.GrowFactor = FMath::Max(InConfig.GrowFactor, C_MinGrowFactor);
	
	if (InConfig.AutoReduceTime > 0.f)
	{
		InConfig.AutoReduceTime = FMath::Max(InConfig.AutoReduceTime, C_MinAutoReduceTime);
	}
	
	Config = InConfig;
	Capacity = 0;

	const int32 RealSize = FMath::Max(Config.InitCapacity, C_DefaultCapacity);
	bInit = Expand(RealSize);

	return bInit;
}

bool UObjectPool::InitDefault(const TSoftClassPtr<UObject>& ClassType, UObject* InObjectPoolRoot)
{
	FObjectPoolConfig DefaultConfig;
	DefaultConfig.ClassType = ClassType;
	DefaultConfig.InitCapacity = C_DefaultCapacity;
	DefaultConfig.GrowFactor = C_MinGrowFactor;
	DefaultConfig.AutoReduceTime = 0.f;

	return Init(DefaultConfig, InObjectPoolRoot);
}

bool UObjectPool::Expand(const int32 ExpandSize)
{
	const int32 RealExpandSize = FMath::Max(ExpandSize, C_MinGrowSize);
	if (RealExpandSize <= 0)
	{
		return false;
	}
	
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UOBJECTPOOL_EXPAND);
	
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
		UObject* NewSpawnObject = NewObject<UObject>(ObjectPoolRoot, ClassType, FName(RecycleName));
		if (!NewSpawnObject)
		{
			return false;
		}

		new (UnusedObjects) FObjectPoolItemWrapper(NewSpawnObject, Config.AutoReduceTime);

		UE_LOG(LogObjectPool, VeryVerbose, TEXT("UObjectPool::Expand Expand '%s' Success For Type:%s PoolSize:%d Capacity:%d"),
			*RecycleName, *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);
	}
	
	return true;
}

bool UObjectPool::CheckBufferOnSpawn()
{
	for (int32 Index = UnusedObjects.Num() - 1; Index >= 0; Index--)
	{
		// It maybe gc outside, just remove invalid items here
		if (!IsValid(UnusedObjects[Index].Obj))
		{
			UnusedObjects.RemoveAtSwap(Index);
		}
		// Find one is valid
		else
		{
			break;
		}
	}

	// Expand if buffer is empty
	if (Empty())
	{
		int32 RealCapacity = Capacity;
		if (RealCapacity <= 0)
		{
			RealCapacity = C_MinCapacity > 0 ? C_MinCapacity : 4.0f;
		}
		
		const int32 ExpandCount = RealCapacity * (Config.GrowFactor - 1);

		if (!Expand(ExpandCount))
		{
			UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Spawn Failed to expand pool for Type:%s !"), *(Config.ClassType.ToString()));
			return false;
		}
	}	

	return true;
}

UObject* UObjectPool::Spawn(UObject* InOuter, const FOnSpawnObjectFromPoolDelegate OnSpawnObjectFromPool/* = FOnSpawnObjectFromPoolDelegate()*/)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UOBJECTPOOL_SPAWN);
	
	if (!bInit || bPendingRemove)
	{
		return nullptr;
	}
	
	if (!Config.ClassType.IsValid())
	{
		UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Spawn ObjectType:%s is invalid!"), *(Config.ClassType.ToString()));
		return nullptr;
	}
	
	if (!CheckBufferOnSpawn())
	{
		UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Spawn CheckBufferOnSpawn failed for Type:%s !"), *(Config.ClassType.ToString()));
		return nullptr;
	}
	
	UObject* SpawnObject = UnusedObjects.Pop().Obj;
	if (!IsValid(SpawnObject))
	{
		UE_LOG(LogObjectPool, Error, TEXT("UObjectPool::Spawn Failed to spawn a object(Type:%s) from object pool!"), *(Config.ClassType.ToString()));
		return nullptr;
	}

	const FString SpawnName = SpawnObject->GetName().Replace(TEXT("Recycled"), TEXT("Spawned"));
	SpawnObject->Rename(*SpawnName, InOuter, REN_ForceNoResetLoaders);

	UE_LOG(LogObjectPool, VeryVerbose, TEXT("UObjectPool::Spawn Spawn '%s' Success For Type:%s PoolSize:%d Capacity:%d"),
		*SpawnName, *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);

	OnSpawnObjectFromPool.ExecuteIfBound(SpawnObject);

	IReusable* ReusableObject = Cast<IReusable>(SpawnObject);
	if (ReusableObject)
	{
		ReusableObject->SpawnFromPool();
	}

	return SpawnObject;
}

bool UObjectPool::Recycle(UObject* RecycleObject)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UOBJECTPOOL_RECYCLE);
	
	if (!bInit || bPendingRemove)
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
	
	new (UnusedObjects) FObjectPoolItemWrapper(RecycleObject, Config.AutoReduceTime);

	IReusable* ReusableObject = Cast<IReusable>(RecycleObject);
	if (ReusableObject)
	{
		ReusableObject->RecycleToPool();
	}

	const FString RecycleName = RecycleObject->GetName().Replace(TEXT("Spawned"), TEXT("Recycled"));
	// - Set 'ObjectPoolRoot' as current's outer avoiding gc (Outer may be set outside, such as streaming level etc.)
	// - Set flag 'REN_ForceNoResetLoaders' to avoid 'FlushAsyncLoading'
	// - Set outer after 'ProcessEvent' to avoid failing to call 'RecycleToPool'
	RecycleObject->Rename(*RecycleName, ObjectPoolRoot, REN_ForceNoResetLoaders);

	UE_LOG(LogObjectPool, VeryVerbose, TEXT("UObjectPool::Recycle Recycle '%s' Success For Type:%s PoolSize:%d Capacity:%d"),
		*RecycleName, *(Config.ClassType->GetName()), UnusedObjects.Num(), Capacity);
	
	return true;
}

void UObjectPool::RemovePoolItems(TArray<FObjectPoolItemWrapper>& PendingRemoveArray)
{
	if (PendingRemoveArray.Num() <= 0)
	{
		return;
	}
	
	for (FObjectPoolItemWrapper& PoolItem : PendingRemoveArray)
	{
		// Handle Actor
		AActor* ActorObject = Cast<AActor>(PoolItem.Obj);
		if (IsValid(ActorObject) && ActorObject->IsActorInitialized())
		{
			ActorObject->Destroy();
		}

		// It will be GC later
		PoolItem.Obj = nullptr;
	}
}

void UObjectPool::Tick(const float DeltaSeconds)
{
	if (!bInit)
	{
		return;
	}

	QUICK_SCOPE_CYCLE_COUNTER(STAT_UOBJECTPOOL_TICK);
	
	for (int32 Index = UnusedObjects.Num() - 1; Index > -1; Index--)
	{
		FObjectPoolItemWrapper& PoolItem = UnusedObjects[Index];
		if (PoolItem.Tick(DeltaSeconds))
		{
			new (PendingRemoveObjects) FObjectPoolItemWrapper(PoolItem);
			UnusedObjects.RemoveAtSwap(Index);

			bPendingRemove = true;
		}
	}

	if (bPendingRemove)
	{
		Capacity -= PendingRemoveObjects.Num();
		
		RemovePoolItems(PendingRemoveObjects);

		UE_LOG(LogObjectPool, VeryVerbose, TEXT("UObjectPool::Tick AutoReduce Type:%s ReduceSize:%d PoolSize:%d Capacity:%d"),
			*(Config.ClassType->GetName()), PendingRemoveObjects.Num(), UnusedObjects.Num(), Capacity);

		bPendingRemove = false;

		// Force GC to save memory immediately if C_ForceGCAfterReduce set true, otherwise use system auto GC
		if (C_ForceGCAfterReduce)
		{
			GEngine->ForceGarbageCollection();
		}
	}

	PendingRemoveObjects.Empty(C_MinCapacity);	
}

void UObjectPool::Clear()
{
	if (!bInit)
	{
		return;
	}

	QUICK_SCOPE_CYCLE_COUNTER(STAT_UOBJECTPOOL_CLEAR);

	RemovePoolItems(PendingRemoveObjects);
	PendingRemoveObjects.Empty();

	RemovePoolItems(UnusedObjects);
	UnusedObjects.Empty();
	
	// Config.Reset();
	Capacity = 0;
}