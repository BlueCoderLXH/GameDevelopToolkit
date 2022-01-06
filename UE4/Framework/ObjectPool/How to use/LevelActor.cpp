AActor* UWorld::SpawnActor( UClass* Class, FTransform const* UserTransformPtr, const FActorSpawnParameters& SpawnParameters )
{
	...

	AActor* Actor = nullptr;
	
	// Try to spawn actor from pool if necessary
	do
	{	
		UWorld* World = LevelToSpawnIn->GetWorld();
		if (!World || !World->bUseObjectPool)
		{
			break;
		}

		if (!Class->ImplementsInterface(UReusable::StaticClass()))
		{
			break;
		}
		
		UObjectPoolSystem* PoolSystem = World->GetObjectPoolSystem();
		if (!PoolSystem)
		{
			break;
		}

		Actor = Cast<AActor>(PoolSystem->Spawn(Class, LevelToSpawnIn));
		if (!Actor)
		{
			UE_LOG(LogSpawn, Error, TEXT("Failed to spawn actor from pool for type:%s"), *(Class->GetFullName()));
			break;
		}

		if (NewActorName != NAME_None)
		{
			Actor->Rename(*(NewActorName.ToString()));
		}

		Actor->SetOwner(SpawnParameters.Owner);

		IReusable::Execute_OnSpawn(Actor);
		
		Actor->SetActorTransform(UserTransform);

		if (ExternalPackage)
		{
			Actor->SetExternalPackage(ExternalPackage);
		}

		if (Actor->IsActorInitialized())
		{
			return Actor;
		}
	}
	while (false);

	if (!Actor)
	{
		// actually make the actor object
		Actor = NewObject<AActor>(LevelToSpawnIn, Class, NewActorName, ActorFlags, Template,false/*bCopyTransientsFromClassDefaults*/, nullptr/*InInstanceGraph*/, ExternalPackage);
	}	
	
	...
}

bool UWorld::DestroyActor( AActor* ThisActor, bool bNetForce, bool bShouldModifyLevel )
{
	...

	// Try to recycle actor to pool if necessary
	do
	{
		UWorld* World = ThisActor->GetWorld(); 
		if (!World || !World->bUseObjectPool)
		{
			break;
		}
		
		UClass* ActorClass = ThisActor->GetClass();
		if (!ActorClass)
		{
			break;
		}

		if (!ActorClass->ImplementsInterface(UReusable::StaticClass()))
		{
			break;
		}

		UObjectPoolSystem* PoolSystem = World->GetObjectPoolSystem();
		if (!PoolSystem)
		{
			break;
		}

		if (PoolSystem->Recycle(ThisActor))
		{
			IReusable::Execute_OnRecycle(ThisActor);
			return true;
		}
	}
	while (false);

	...
}