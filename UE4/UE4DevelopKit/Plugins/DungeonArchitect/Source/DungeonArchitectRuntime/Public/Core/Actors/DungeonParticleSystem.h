//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonParticleSystem.generated.h"

class UParticleSystemComponent;
class USceneComponent;
/**
*
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API ADungeonParticleSystem : public AActor {
    GENERATED_UCLASS_BODY()

public:
    void SetParticleComponentFromTemplate(UParticleSystemComponent* Template);

private:
    UParticleSystemComponent* ParticleSystem;
    USceneComponent* SceneRoot;
};

