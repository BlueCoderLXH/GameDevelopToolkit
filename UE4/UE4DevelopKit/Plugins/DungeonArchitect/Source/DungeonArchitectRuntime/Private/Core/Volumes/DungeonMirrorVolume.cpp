//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Volumes/DungeonMirrorVolume.h"

#include "Components/BrushComponent.h"
#include "Engine/CollisionProfile.h"

ADungeonMirrorVolume::ADungeonMirrorVolume(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    UBrushComponent* BrushComp = GetBrushComponent();
    if (BrushComp) {
        BrushComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
        BrushComp->SetGenerateOverlapEvents(false);
    }
}

