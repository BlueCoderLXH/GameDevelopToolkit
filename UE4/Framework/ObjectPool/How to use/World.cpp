void UWorld::InitObjectPool()
{
	if (!bUseObjectPool || ObjectPoolSystem)
	{
		return;
	}

	ObjectPoolSystem = NewObject<UObjectPoolSystem>(this);

	if (ObjectPoolSystem)
	{
		ObjectPoolSystem->Init();
	}
	else
	{
		UE_LOG(LogWorld, Error, TEXT("UWorld::InitObjectPool Failed to initialize the object pool system"));
	}
}

void UWorld::BeginPlay()
{
	...
	
	if (bUseObjectPool)
	{
		InitObjectPool();
	}

	...
}

void UWorld::BeginDestroy()
{
	...
	
	if (ObjectPoolSystem)
	{
		ObjectPoolSystem->Clear();
		ObjectPoolSystem = nullptr;
	}

	...
}