// #include "Reusable.h"
//
// extern OBJECTPOOL_API TAutoConsoleVariable<bool> CVarEnableObjectPool;
//
// int64 UActorChannel::Close(EChannelCloseReason Reason)
// {
// 	FScopedRepContext RepContext(Connection, Actor);
//
// 	UE_LOG(LogNetTraffic, Log, TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s, Reason: %s"), ChIndex, *GetFullNameSafe(Actor), LexToString(Reason));
// 	int64 NumBits = UChannel::Close(Reason);
//
// 	if (Actor != nullptr)
// 	{
// 		bool bKeepReplicators = false;		// If we keep replicators around, we can use them to determine if the actor changed since it went dormant
//
// 		if (Connection)
// 		{
// 			if (Reason == EChannelCloseReason::Dormancy)
// 			{
// 				const bool bIsDriverValid = Connection->Driver != nullptr;
// 				const bool bIsServer = bIsDriverValid && Connection->Driver->IsServer();
// 				if (bIsDriverValid)
// 				{
// 					if (!bIsServer)
// 					{
// 						Actor->NetDormancy = DORM_DormantAll;
// 					}
//
// 					check( Actor->NetDormancy > DORM_Awake ); // Dormancy should have been canceled if game code changed NetDormancy
// 					Connection->Driver->NotifyActorFullyDormantForConnection(Actor, Connection);
// 				}
//
// 				// Validation checking
// 				// We need to keep the replicators around so we can reuse them.
// 				bKeepReplicators = (GNetDormancyValidate > 0) || (bIsServer && GbNetReuseReplicatorsForDormantObjects);
// 			}
// 			// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ObjectPool Start
// 			else if (Reason == EChannelCloseReason::Relevancy || Reason == EChannelCloseReason::Destroyed)
// 			{
// 				const bool bIsDriverValid = Connection->Driver != nullptr;
// 				const bool bIsServer = bIsDriverValid && Connection->Driver->IsServer();
//
// 				// Reset object-pooled actor's rep-components' guid ack status on server side (by lxh)
// 				//
// 				// On client side, GuidCache for object-pooled actor's rep-components has been removed when this actor is destroyed(ChannelCloseReason:Relevancy)
// 				// When this actor coming to be active, DS won't replicate rep-components' net guid for it has been acked, so that client won't know what object it is.
// 				// It Will lead to some network problem such as failing to call rpc / replicating properties... 
// 				// To solve this problem, we have to reset object-pooled actor's rep-components' guid ack status here.
// 				bool bShouldHandleForObjectPool = false;
// 				if (IsValid(Actor) && IsValid(Actor->GetClass()) && Actor->Implements<UReusable>())
// 				{
// 					if (const IReusable* DefaultReusableActor = Cast<IReusable>(Actor->GetClass()->GetDefaultObject()))
// 					{
// 						bShouldHandleForObjectPool = DefaultReusableActor->ShouldUseObjectPool();
// 					}
// 				}
// 				
// 				const TSharedPtr<FNetGUIDCache>& GuidCache = bIsServer ? Connection->Driver->GuidCache : nullptr;
// 				if (bIsServer && CVarEnableObjectPool.GetValueOnGameThread() && bShouldHandleForObjectPool && GuidCache.IsValid())
// 				{
// 					const TArray<UActorComponent*>& RepComps = Actor->GetReplicatedComponents();
// 					for (UActorComponent* RepComp : RepComps)
// 					{
// 						if (!IsValid(RepComp)) continue;
// 						
// 						const UPackageMapClient* PackageMapClient = Connection ? Cast<UPackageMapClient>( Connection->PackageMap ) : nullptr;
// 						if (PackageMapClient)
// 						{
// 							PackageMapClient->ResetGuidAckStatus(RepComp);
// 						}
// 					}
// 				}
// 			}
// 			// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ObjectPool End
//
// 			// SetClosingFlag() might have already done this, but we need to make sure as that won't get called if the connection itself has already been closed
// 			Connection->RemoveActorChannel( Actor );
// 		}
//
// 		Actor = nullptr;
// 		CleanupReplicators( bKeepReplicators );
// 	}
//
// 	return NumBits;
// }
//
// bool UActorChannel::CleanUp(const bool bForDestroy, EChannelCloseReason CloseReason)
// {
// 	SCOPE_CYCLE_COUNTER(Stat_ActorChanCleanUp);
//
// 	checkf(Connection != nullptr, TEXT("UActorChannel::CleanUp: Connection is null!"));
// 	checkf(Connection->Driver != nullptr, TEXT("UActorChannel::CleanUp: Connection->Driver is null!"));
//
// 	Connection->Driver->NotifyActorChannelCleanedUp(this, CloseReason);
//
// 	const bool bIsServer = Connection->Driver->IsServer();
//
// 	UE_LOG( LogNetTraffic, Log, TEXT( "UActorChannel::CleanUp: %s" ), *Describe() );
// 	
// 	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ObjectPool Start
// 	if (!bIsServer && CVarEnableObjectPool.GetValueOnGameThread())
// 	{
// 		bool bShouldHandleForObjectPool = false;
// 		if (IsValid(Actor) && IsValid(Actor->GetClass()) && Actor->Implements<UReusable>())
// 		{
// 			if (const IReusable* DefaultReusableActor = Cast<IReusable>(Actor->GetClass()->GetDefaultObject()))
// 			{
// 				bShouldHandleForObjectPool = DefaultReusableActor->ShouldUseObjectPool();
// 			}
// 		}
// 		
// 		// Remove object-pooled actor from global cache on client side (by lxh)
// 		//
// 		// The object-pooled actor and its replicated components must be removed from 'ObjectLookup' and 'NetGUIDLookup' in GuidCache
// 		// To ensure the exact object pool flow for actors(SpawnActor and DestroyActor).
// 		if (bShouldHandleForObjectPool)
// 		{
// 			FNetGUIDCache* GuidCache = Connection->Driver->GuidCache.Get();
// 			const FNetworkGUID* ObjNetGuid = GuidCache->NetGUIDLookup.Find(Actor);
// 			if (ObjNetGuid)
// 			{
// 				GuidCache->NetGUIDLookup.Remove(Actor);
// 				GuidCache->ObjectLookup.Remove(*ObjNetGuid);
// 			}
//
// 			const TArray<UActorComponent*>& RepComps = Actor->GetReplicatedComponents();
// 			for (UActorComponent* RepComp : RepComps)
// 			{
// 				const FNetworkGUID* RepCompNetGuid = GuidCache->NetGUIDLookup.Find(RepComp);
// 				if (RepCompNetGuid)
// 				{
// 					GuidCache->NetGUIDLookup.Remove(RepComp);
// 					GuidCache->ObjectLookup.Remove(*RepCompNetGuid);
// 				}
// 			}
// 		}
// 	}
// 	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ObjectPool End
//
// 	if (!bIsServer && QueuedBunches.Num() > 0 && ChIndex >= 0 && !bForDestroy)
// 	{
// 		checkf(ActorNetGUID.IsValid(), TEXT("UActorChannel::Cleanup: ActorNetGUID is invalid! Channel: %i"), ChIndex);
// 		
// 		TArray<UActorChannel*>& ChannelsStillProcessing = Connection->KeepProcessingActorChannelBunchesMap.FindOrAdd(ActorNetGUID);
// 		
// #if DO_CHECK
// 		if (ensureMsgf(!ChannelsStillProcessing.Contains(this), TEXT("UActorChannel::CleanUp encountered a channel already within the KeepProcessingActorChannelBunchMap. Channel: %i"), ChIndex))
// #endif // #if DO_CHECK
// 		{
// 			UE_LOG(LogNet, VeryVerbose, TEXT("UActorChannel::CleanUp: Adding to KeepProcessingActorChannelBunchesMap. Channel: %i, Num: %i"), ChIndex, Connection->KeepProcessingActorChannelBunchesMap.Num());
//
// 			// Remember the connection, since CleanUp below will NULL it
// 			UNetConnection* OldConnection = Connection;
//
// 			// This will unregister the channel, and make it free for opening again
// 			// We need to do this, since the server will assume this channel is free once we ack this packet
// 			Super::CleanUp(bForDestroy, CloseReason);
//
// 			// Restore connection property since we'll need it for processing bunches (the Super::CleanUp call above NULL'd it)
// 			Connection = OldConnection;
//
// 			QueuedCloseReason = CloseReason;
//
// 			// Add this channel to the KeepProcessingActorChannelBunchesMap list
// 			ChannelsStillProcessing.Add(this);
//
// 			// We set ChIndex to -1 to signify that we've already been "closed" but we aren't done processing bunches
// 			ChIndex = -1;
//
// 			// Return false so we won't do pending kill yet
// 			return false;
// 		}
// 	}
//
// 	bool bWasDormant = false;
//
// 	// If we're the client, destroy this actor.
// 	if (!bIsServer)
// 	{
// 		check(Actor == NULL || Actor->IsValidLowLevel());
// 		checkSlow(Connection->IsValidLowLevel());
// 		checkSlow(Connection->Driver->IsValidLowLevel());
// 		if (Actor != NULL)
// 		{
// 			if (Actor->GetTearOff() && !Connection->Driver->ShouldClientDestroyTearOffActors())
// 			{
// 				if (!bTornOff)
// 				{
// 					Actor->SetRole(ROLE_Authority);
// 					Actor->SetReplicates(false);
// 					bTornOff = true;
// 					if (Actor->GetWorld() != NULL && !IsEngineExitRequested())
// 					{
// 						Actor->TornOff();
// 					}
//
// 					Connection->Driver->NotifyActorTornOff(Actor);
// 				}
// 			}
// 			else if (Dormant && (CloseReason == EChannelCloseReason::Dormancy) && !Actor->GetTearOff())	
// 			{
// 				Actor->NetDormancy = DORM_DormantAll;
//
// 				Connection->Driver->NotifyActorFullyDormantForConnection(Actor, Connection);
// 				bWasDormant = true;
// 			}
// 			else if (!Actor->bNetTemporary && Actor->GetWorld() != NULL && !IsEngineExitRequested() && Connection->Driver->ShouldClientDestroyActor(Actor))
// 			{
// 				UE_LOG(LogNetDormancy, Verbose, TEXT("UActorChannel::CleanUp: Destroying Actor. %s"), *Describe() );
//
// 				DestroyActorAndComponents();
// 			}
// 		}
// 	}
//
// 	// Remove from hash and stuff.
// 	SetClosingFlag();
//
// 	// If this actor is going dormant (and we are a client), keep the replicators around, we need them to run the business logic for updating unmapped properties
// 	const bool bKeepReplicators = !bForDestroy && bWasDormant && (!bIsServer || GbNetReuseReplicatorsForDormantObjects);
//
// 	CleanupReplicators( bKeepReplicators );
//
// 	// We don't care about any leftover pending guids at this point
// 	PendingGuidResolves.Empty();
// 	QueuedBunchObjectReferences.Empty();
//
// 	// Free export bunches list
// 	for (FOutBunch* QueuedOutBunch : QueuedExportBunches)
// 	{
// 		delete QueuedOutBunch;
// 	}
//
// 	QueuedExportBunches.Empty();
//
// 	// Free the must be mapped list
// 	QueuedMustBeMappedGuidsInLastBunch.Empty();
//
// 	if (QueuedBunches.Num() > 0)
// 	{
// 		// Free any queued bunches
// 		for (FInBunch* QueuedInBunch : QueuedBunches)
// 		{
// 			delete QueuedInBunch;
// 		}
//
// 		QueuedBunches.Empty();
//
// 		if (UPackageMapClient* PackageMapClient = Cast<UPackageMapClient>(Connection->PackageMap))
// 		{
// 			PackageMapClient->SetHasQueuedBunches(ActorNetGUID, false);
// 		}
// 	}
//
// 	// We check for -1 here, which will be true if this channel has already been closed but still needed to process bunches before fully closing
// 	if (ChIndex >= 0)	
// 	{
// 		return Super::CleanUp(bForDestroy, CloseReason);
// 	}
// 	else
// 	{
// 		// Because we set Connection = OldConnection; above when we set ChIndex to -1, we have to null it here explicitly to make sure the connection is cleared by the time we leave CleanUp
// 		Connection = nullptr;
// 	}
//
// 	return true;
// }