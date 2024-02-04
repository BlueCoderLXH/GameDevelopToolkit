#include "Reusable.h"

extern ENGINE_API TAutoConsoleVariable<bool> CVarEnableObjectPool;

void UNetDriver::NotifyActorDestroyed( AActor* ThisActor, bool IsSeamlessTravel )
{
	// Remove the actor from the property tracker map
	RepChangedPropertyTrackerMap.Remove(ThisActor);

	const bool bIsServer = IsServer();
	
	if (bIsServer)
	{
		FActorDestructionInfo* DestructionInfo = nullptr;

		const bool bIsActorStatic = !GuidCache->IsDynamicObject( ThisActor );
		const bool bActorHasRole = ThisActor->GetRemoteRole() != ROLE_None;
		const bool bShouldCreateDestructionInfo = bIsServer && bIsActorStatic && bActorHasRole && !IsSeamlessTravel;

		if (bShouldCreateDestructionInfo)
		{
			UE_LOG(LogNet, VeryVerbose, TEXT("NotifyActorDestroyed %s - StartupActor"), *ThisActor->GetPathName() );
			DestructionInfo = CreateDestructionInfo( this, ThisActor, DestructionInfo);
		}

		const FNetworkObjectInfo* NetworkObjectInfo = GetNetworkObjectList().Find( ThisActor ).Get();

		for( int32 i=ClientConnections.Num()-1; i>=0; i-- )
		{
			UNetConnection* Connection = ClientConnections[i];
			if( ThisActor->bNetTemporary )
				Connection->SentTemporaries.Remove( ThisActor );
			UActorChannel* Channel = Connection->FindActorChannelRef(ThisActor);
			if( Channel )
			{
				check(Channel->OpenedLocally);
				Channel->bClearRecentActorRefs = false;
				Channel->Close(EChannelCloseReason::Destroyed);
			}
			else
			{
				const bool bDormantOrRecentlyDormant = NetworkObjectInfo && (NetworkObjectInfo->DormantConnections.Contains(Connection) || NetworkObjectInfo->RecentlyDormantConnections.Contains(Connection));

				if (bShouldCreateDestructionInfo || bDormantOrRecentlyDormant)
				{
					// Make a new destruction info if necessary. It is necessary if the actor is dormant or recently dormant because
					// even though the client knew about the actor at some point, it doesn't have a channel to handle destruction.
					DestructionInfo = CreateDestructionInfo(this, ThisActor, DestructionInfo);
					if (DestructionInfo)
					{
						Connection->AddDestructionInfo(DestructionInfo);
					}
				}
			}

			Connection->NotifyActorDestroyed(ThisActor);
		}

		// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>OBJECTPOOL START
		// Remove object-pooled actor from global cache on server side (by lxh)
		//
		// The object-pooled actor and its replicated components must be removed from 'ObjectLookup' and 'NetGUIDLookup' in GuidCache
		// To ensure the exact object pool flow for actors(SpawnActor and DestroyActor).
		bool bShouldHandleForObjectPool = false;
		if (IsValid(ThisActor) && IsValid(ThisActor->GetClass()) && ThisActor->Implements<UReusable>())
		{
			if (const IReusable* DefaultReusableActor = Cast<IReusable>(ThisActor->GetClass()->GetDefaultObject()))
			{
				bShouldHandleForObjectPool = DefaultReusableActor->ShouldUseObjectPool();
			}
		}
		
		if (CVarEnableObjectPool.GetValueOnGameThread() && bShouldHandleForObjectPool && GuidCache.IsValid())
		{
			const FNetworkGUID* ActorNetGuid = GuidCache->NetGUIDLookup.Find(ThisActor);
			if (ActorNetGuid)
			{
				GuidCache->ObjectLookup.Remove(*ActorNetGuid);
				GuidCache->NetGUIDLookup.Remove(ThisActor);
			}

			TArray<UObject*> RepComps;
			ThisActor->GetDefaultSubobjects(RepComps);
			for (UObject* RepComp : RepComps)
			{
				if (!IsValid(RepComp)) continue;

				const FNetworkGUID* RepCompNetGuid = GuidCache->NetGUIDLookup.Find(RepComp);
				if (RepCompNetGuid)
				{
					GuidCache->ObjectLookup.Remove(*RepCompNetGuid);
					GuidCache->NetGUIDLookup.Remove(RepComp);
				}
			}
		}
		// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>OBJECTPOOL END
	}

	if (ServerConnection)
	{
		ServerConnection->NotifyActorDestroyed(ThisActor);
	}

	// Remove this actor from the network object list
	RemoveNetworkActor( ThisActor );
}