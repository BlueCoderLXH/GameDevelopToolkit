//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Snap/Lib/Serialization/SnapActorSerialization.h"

#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnapMapStreamingData, Log, All);

void USnapStreamingChunkActorData::SaveLevel(ULevel* InLevel) {
    if (!InLevel) return;

    ActorEntries.Empty();

    int32 NumActorsSaved = 0;
    for (AActor* Actor : InLevel->Actors) {

        if (UKismetSystemLibrary::DoesImplementInterface(Actor, USnapSerializable::StaticClass())) {
            const int32 EntryIndex = ActorEntries.AddDefaulted();
            FSnapChunkActorDataEntry& Entry = ActorEntries[EntryIndex];
            SaveActor(Actor, Entry);
            NumActorsSaved++;
        }
    }

    UE_LOG(LogSnapMapStreamingData, Log, TEXT("Saved %d actors"), NumActorsSaved);

    Modify();
}

void USnapStreamingChunkActorData::LoadLevel(ULevel* InLevel) {
    if (!InLevel) return;

    // Build a map for faster access
    TMap<FName, const FSnapChunkActorDataEntry*> ActorToEntryMap;
    for (const FSnapChunkActorDataEntry& Entry : ActorEntries) {
        FName Key = *Entry.ActorName;
        if (!ActorToEntryMap.Contains(Key)) {
            ActorToEntryMap.Add(Key, &Entry);
        }
    }

    int32 NumActorsLoaded = 0;
    for (AActor* Actor : InLevel->Actors) {
        if (UKismetSystemLibrary::DoesImplementInterface(Actor, USnapSerializable::StaticClass())) {
            FName Key = Actor->GetFName();
            const FSnapChunkActorDataEntry** SearchResult = ActorToEntryMap.Find(Key);
            if (SearchResult) {
                const FSnapChunkActorDataEntry* EntryPtr = *SearchResult;
                if (EntryPtr->ActorClass == Actor->GetClass()) {
                    LoadActor(Actor, *EntryPtr);
                    NumActorsLoaded++;
                }

                // TODO: Handle deletion / spawning of actors not in the list
            }
        }
    }

    UE_LOG(LogSnapMapStreamingData, Log, TEXT("Deserialized %d actors"), NumActorsLoaded);

}

struct FSnapMapSaveChunkArchive : public FObjectAndNameAsStringProxyArchive {
    FSnapMapSaveChunkArchive(FArchive& InInnerArchive)
        : FObjectAndNameAsStringProxyArchive(InInnerArchive, true) {
        ArIsSaveGame = true;
    }
};

void USnapStreamingChunkActorData::SaveActor(AActor* InActor, FSnapChunkActorDataEntry& OutEntry) {
    OutEntry.ActorName = InActor->GetName();
    OutEntry.ActorClass = InActor->GetClass();
    OutEntry.ActorTransform = InActor->GetActorTransform();

    // https://answers.unrealengine.com/questions/35618/savingloading-an-array-of-objects.html
    FMemoryWriter MemoryWriter(OutEntry.ActorData, true);
    FSnapMapSaveChunkArchive Ar(MemoryWriter);

    InActor->Serialize(Ar);
}

void USnapStreamingChunkActorData::LoadActor(AActor* InActor, const FSnapChunkActorDataEntry& InEntry) {
    if (!InActor) return;
    check(InActor->GetName() == InEntry.ActorName);
    check(InActor->GetClass() == InEntry.ActorClass);
    if (InActor->GetRootComponent() && InActor->GetRootComponent()->Mobility == EComponentMobility::Movable) {
        InActor->SetActorTransform(InEntry.ActorTransform);
    }

    FMemoryReader Reader(InEntry.ActorData, true);
    FSnapMapSaveChunkArchive Ar(Reader);
    InActor->Serialize(Ar);

    ISnapSerializable::Execute_OnSnapDataLoaded(InActor);
}


///////////////////////////////// USnapMapSerializable ///////////////////////////////// 

