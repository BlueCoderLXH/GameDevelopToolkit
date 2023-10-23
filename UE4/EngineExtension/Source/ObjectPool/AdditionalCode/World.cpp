extern OBJECTPOOL_API TAutoConsoleVariable<bool> CVarEnableObjectPool;

void UWorld::InitObjectPool()
{
	if (!CVarEnableObjectPool.GetValueOnGameThread())
	{
		return;
	}	
	
	if (!bUseObjectPool || ObjectPoolSystem)
	{
		return;
	}

	ObjectPoolSystem = NewObject<UObjectPoolSystem>(this->GetCurrentLevel());

	if (ObjectPoolSystem)
	{
		ObjectPoolSystem->Init();
	}
	else
	{
		UE_LOG(LogWorld, Error, TEXT("UWorld::InitObjectPool Failed to initialize the object pool system"));
	}
}

void UWorld::TickObjectPool(const float DeltaSeconds) const
{
	if (ObjectPoolSystem)
	{
		ObjectPoolSystem->Tick(DeltaSeconds);
	}
}

void UWorld::DestroyObjectPool()
{
	if (ObjectPoolSystem)
	{
		ObjectPoolSystem->Clear();
		ObjectPoolSystem = nullptr;
	}	
}

void UWorld::BeginPlay()
{
	AGameModeBase* const GameMode = GetAuthGameMode();
	if (GameMode)
	{
		GameMode->StartPlay();
		if (GetAISystem())
		{
			GetAISystem()->StartPlay();
		}
	}

	OnWorldBeginPlay.Broadcast();

#if WITH_CHAOS
	if(PhysicsScene)
	{
		PhysicsScene->OnWorldBeginPlay();
	}
#endif

	// Init ObjectPool on the end of World::BeginPlay for avoiding call the 'BeginPlay' of new-pooled actors
	// See AWorldSettings::NotifyBeginPlay & FActorIteratorState::FActorIteratorState() for more
	if (bUseObjectPool)
	{
		InitObjectPool();
	}
}

void UWorld::BeginDestroy()
{
	...
	
	DestroyObjectPool();

	...
}