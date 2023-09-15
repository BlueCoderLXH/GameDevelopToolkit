//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Volumes/DungeonThemeOverrideVolume.h"

#include "Components/BrushComponent.h"
#include "Engine/CollisionProfile.h"

ADungeonThemeOverrideVolume::ADungeonThemeOverrideVolume(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer), Reversed(false) {
    UBrushComponent* BrushComp = GetBrushComponent();
    if (BrushComp) {
        BrushComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
        BrushComp->SetGenerateOverlapEvents(false);
    }
}

