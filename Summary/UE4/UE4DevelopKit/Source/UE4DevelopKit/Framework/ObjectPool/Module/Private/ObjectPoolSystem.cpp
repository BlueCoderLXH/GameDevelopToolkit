// #include "ObjectPoolSystem.h"
//
// static bool GEnableObjectPool = true;
// TAutoConsoleVariable<bool> CVarEnableObjectPool(
// 	TEXT("g.EnableObjectPool"),
// 	GEnableObjectPool,
// 	TEXT("Whether to enable object pool, true:enable ; false:disable"),
// 	ECVF_Default
// );
//
// void UObjectPoolSystem::Init()
// {
// 	if (!CVarEnableObjectPool.GetValueOnGameThread())
// 	{
// 		return;
// 	}
// 	
// 	if (ConfigArray.Num() <= 0)
// 	{
// 		return;
// 	}
//
// 	if (!GetOuter())
// 	{
// 		return;
// 	}
// 	
// 	bInit = true;
//
// 	const FString& OuterName =  GetOuter()->GetName();
//
// 	for (const FObjectPoolConfig& PoolCfgItem : ConfigArray)
// 	{
// 		if (CanLevelUseObjectPool(OuterName, PoolCfgItem.ExcludeLevelNames))
// 		{
// 			Add(PoolCfgItem);
// 		}
// 	}
//
// 	UE_LOG(LogObjectPool, Verbose, TEXT("UObjectPoolSystem::Init ObjectPools:%d In World:%s!"), PoolMap.Num(), *(GetOuter()->GetName()));
// }
//
// bool UObjectPoolSystem::CanLevelUseObjectPool(const FString& LevelName, const FString& ExcludeLevelNames)
// {
// 	FString Left = ExcludeLevelNames;
// 	FString Right = "";
// 	
// 	while (!Left.IsEmpty())
// 	{
// 		if (!Left.Split(";", &Left, &Right))
// 		{
// 			Right = "";
// 		}
// 		
// 		if (Left == LevelName)
// 		{
// 			return false;
// 		}
// 		
// 		Left = Right;
// 	}
//
// 	return true;
// }
//
// bool UObjectPoolSystem::CommonCheck(const TSoftClassPtr<UObject> ClassType)
// {
// 	if (ClassType.IsNull())
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::CommonCheck ObjectType is invald!"));
// 		return false;
// 	}
//
// 	const auto ClassTypePtr = ClassType.LoadSynchronous();
// 	if (!ClassTypePtr || !ClassTypePtr->ImplementsInterface(UReusable::StaticClass()))
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::CommonCheck ObjectType:%s doesn't implement IReusable or is invalid!"),
// 			*(ClassType.ToString()));
// 		return false;
// 	}
//
// 	return true;
// }
//
// void UObjectPoolSystem::Add(const FObjectPoolConfig& PoolConfig)
// {
// 	AddInner(PoolConfig);
// }
//
// void UObjectPoolSystem::AddDefault(const TSoftClassPtr<UObject> ClassType)
// {
// 	FObjectPoolConfig PoolConfig;
// 	PoolConfig.ClassType = ClassType;
// 	
// 	AddInner(PoolConfig, true);
// }
//
// void UObjectPoolSystem::AddInner(const FObjectPoolConfig& PoolConfig, const bool bMakeDefault)
// {
// 	if (!bInit)
// 	{
// 		return;
// 	}
//
// 	const auto ClassType = PoolConfig.ClassType;
// 	if (!CommonCheck(ClassType))
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::AddInner ObjectType:%s CommonCheck:false!"), *(ClassType.ToString()));
// 		return;
// 	}
// 	
// 	if (PoolMap.Contains(ClassType))
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::AddInner ObjectType:%s already been in the pool!"), *(ClassType.ToString()));
// 		return;
// 	}
//
// 	UObject* InOuter = GetOuter();
// 	UObjectPool* ObjectPool = NewObject<UObjectPool>(InOuter);
// 	if (!ObjectPool)
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::AddInner Failed to create ObjectPool!"));
// 		return;
// 	}
//
// 	if (bMakeDefault)
// 	{
// 		ObjectPool->InitDefault(PoolConfig.ClassType);
// 	}
// 	else
// 	{
// 		ObjectPool->Init(PoolConfig);
// 	}
//
// 	PoolMap.Add(ClassType, ObjectPool);
// }
//
// void UObjectPoolSystem::Remove(const TSoftClassPtr<UObject>& ClassType)
// {
// 	if (!bInit)
// 	{
// 		return;
// 	}
// 	
// 	if (!CommonCheck(ClassType))
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::Remove ObjectType:%s CommonCheck:false!"), *(ClassType.ToString()));
// 		return;
// 	}
//
// 	UObjectPool** ObjectPoolPtr = PoolMap.Find(ClassType);
// 	if (!ObjectPoolPtr)
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::Remove ObjectType:%s isn't in the pool!"), *(ClassType.ToString()));
// 		return;
// 	}
//
// 	(*ObjectPoolPtr)->Clear();
//
// 	PoolMap.Remove(ClassType);
// }
//
// UObject* UObjectPoolSystem::SpawnInner(const TSoftClassPtr<>& ClassType, UObject* InOuter,
// 	const FOnSpawnObjectFromPoolDelegate OnSpawnObjectFromPool)
// {
// 	UObjectPool** ObjectPoolPtr = PoolMap.Find(ClassType);
// 	if (!ObjectPoolPtr)
// 	{
// 		const auto ClassTypePtr = ClassType.LoadSynchronous();
// 		if (!ClassTypePtr->ImplementsInterface(UReusable::StaticClass()))
// 		{
// 			return nullptr;
// 		}
//
// 		/**
// 		 * Auto add default object pool that isn't configured in .ini file but implements 'IReusable'
// 		 * We doesn't suggest this way for it can make lag in runtime!
// 		 */
// 		AddDefault(ClassType);
//
// 		ObjectPoolPtr = PoolMap.Find(ClassType);
// 		if (!ObjectPoolPtr)
// 		{
// 			return nullptr;
// 		}
// 	}
//
// 	return (*ObjectPoolPtr)->Spawn(InOuter, OnSpawnObjectFromPool);
// }
//
// UObject* UObjectPoolSystem::Spawn(const TSoftClassPtr<UObject>& ClassType, UObject* InOuter,
// 	const FOnSpawnObjectFromPoolDelegate OnSpawnObjectFromPool)
// {
// 	return SpawnInner(ClassType, InOuter, OnSpawnObjectFromPool);
// }
//
// UObject* UObjectPoolSystem::Spawn(const TSoftClassPtr<UObject>& ClassType, UObject* InOuter)
// {
// 	return SpawnInner(ClassType, InOuter, FOnSpawnObjectFromPoolDelegate());;
// }
//
// bool UObjectPoolSystem::Recycle(UObject* RecycleObject)
// {
// 	if (!bInit)
// 	{
// 		return false;
// 	}
// 	
// 	if (!IsValid(RecycleObject))
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::Recycle 'RecycleObject' is invalid!"));
// 		return false;
// 	}
// 	
// 	const TSoftClassPtr<UObject> ClassType = RecycleObject->GetClass();
// 	if (!CommonCheck(ClassType))
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::Recycle ObjectType:%s CommonCheck:false!"), *(ClassType.ToString()));
// 		return false;
// 	}
//
// 	UObjectPool** ObjectPoolPtr = PoolMap.Find(ClassType);
// 	if (!ObjectPoolPtr)
// 	{
// 		UE_LOG(LogObjectPool, Error, TEXT("UObjectPoolSystem::Recycle ObjectType:%s isn't in the pool!"), *(ClassType.ToString()));
// 		return false;
// 	}
//
// 	return (*ObjectPoolPtr)->Recycle(RecycleObject);
// }
//
// void UObjectPoolSystem::Clear()
// {
// 	for (auto Item : PoolMap)
// 	{
// 		Item.Value->Clear();
// 	}
// 	
// 	PoolMap.Empty();
// }