//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Actors/DungeonParticleSystem.h"

#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"

ADungeonParticleSystem::ADungeonParticleSystem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
                                                                                              ParticleSystem(nullptr) {
    SceneRoot = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "SceneRoot");
    SceneRoot->SetMobility(EComponentMobility::Static);
    RootComponent = SceneRoot;
}

void ADungeonParticleSystem::SetParticleComponentFromTemplate(UParticleSystemComponent* Template) {
    if (ParticleSystem) {
        ParticleSystem->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepRelative, false));
        ParticleSystem->DestroyComponent();
    }
    ParticleSystem = NewObject<UParticleSystemComponent>(this, TEXT("ParticleSystem"), RF_NoFlags, Template);
    ParticleSystem->SetMobility(EComponentMobility::Stationary);
    ParticleSystem->SetupAttachment(SceneRoot);

    ReregisterAllComponents();
}

