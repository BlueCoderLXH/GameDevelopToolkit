//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "SnapConnectionActorFactory.generated.h"

UCLASS()
class DUNGEONARCHITECTEDITOR_API USnapConnectionActorFactory : public UActorFactory {
    GENERATED_UCLASS_BODY()

    // UActorFactory interface
    virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
    virtual AActor* SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags,
                               FName Name) override;
    virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
    virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
    virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
    // End of UActorFactory interface
};

