AActor* UWorld::SpawnActor( UClass* Class, FTransform const* UserTransformPtr, const FActorSpawnParameters& SpawnParameters )
{
	...

	AActor* Actor = nullptr;
	
	// Try to spawn actor from pool if necessary
	do
	{	
		const UWorld* World = LevelToSpawnIn->GetWorld();
		if (!World || !World->bUseObjectPool)
		{
			break;
		}

		if (!Class->ImplementsInterface(UReusable::StaticClass()))
		{
			break;
		}

		const IReusable* DefaultObjectFromClass = Cast<IReusable>(Class->GetDefaultObject());
		if (!DefaultObjectFromClass || !DefaultObjectFromClass->ShouldUseObjectPool())
		{
			break;
		}
		
		UObjectPoolSystem* PoolSystem = World->GetObjectPoolSystem();
		if (!PoolSystem || !PoolSystem->IsAccessible(Class))
		{
			break;
		}

		auto OnSpawnObjectFromPool = [&] (UObject* SpawnedObject)
		{
			AActor* NewActor = Cast<AActor>(SpawnedObject);
			if (NewActor)
			{
				if (NewActorName != NAME_None)
				{
					NewActor->Rename(*(NewActorName.ToString()));
				}
				
				if (NewActor->IsActorInitialized())
				{
					NewActor->SetOwner(SpawnParameters.Owner);
					NewActor->SetActorTransform(UserTransform);

					if (ExternalPackage)
					{
						NewActor->SetExternalPackage(ExternalPackage);
					}
				}
			}
		};

		Actor = Cast<AActor>(PoolSystem->Spawn(Class, LevelToSpawnIn, FOnSpawnObjectFromPoolDelegate::CreateLambda(OnSpawnObjectFromPool)));
		if (!Actor)
		{
			UE_LOG(LogSpawn, Error, TEXT("Failed to spawn actor from pool for type:%s"), *(Class->GetFullName()));
			break;
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
		const UWorld* World = ThisActor->GetWorld(); 
		if (!World || !World->bUseObjectPool)
		{
			break;
		}
		
		const UClass* ActorClass = ThisActor->GetClass();
		if (!ActorClass)
		{
			break;
		}

		if (!ActorClass->ImplementsInterface(UReusable::StaticClass()))
		{
			break;
		}

		const IReusable* DefaultObjectFromClass = Cast<IReusable>(ActorClass->GetDefaultObject());
		if (!DefaultObjectFromClass || !DefaultObjectFromClass->ShouldUseObjectPool())
		{
			break;
		}

		UObjectPoolSystem* PoolSystem = World->GetObjectPoolSystem();
		if (!PoolSystem || !PoolSystem->IsAccessible(ActorClass))
		{
			break;
		}

		if (PoolSystem->Recycle(ThisActor))
		{
			// Notify net drivers that this guy has been destroyed.
			if (FWorldContext* Context = GEngine->GetWorldContextFromWorld(this))
			{
				for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
				{
					if (Driver.NetDriver != nullptr && Driver.NetDriver->ShouldReplicateActor(ThisActor))
					{
						Driver.NetDriver->NotifyActorDestroyed(ThisActor);
					}
				}
			}
			
			return true;
		}

		// Simply return 'false' if the pooled-object has been destroyed more than once
		return false;
	}
	while (false);

	...
}