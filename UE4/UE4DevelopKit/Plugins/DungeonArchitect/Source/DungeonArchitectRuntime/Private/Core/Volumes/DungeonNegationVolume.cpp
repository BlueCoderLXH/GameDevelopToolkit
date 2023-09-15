//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Volumes/DungeonNegationVolume.h"

#include "Components/BrushComponent.h"
#include "Engine/CollisionProfile.h"

ADungeonNegationVolume::ADungeonNegationVolume(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer), Reversed(false), AffectsUserDefinedCells(true) {
    UBrushComponent* BrushComp = GetBrushComponent();
    if (BrushComp) {
        BrushComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
        BrushComp->SetGenerateOverlapEvents(false);
    }
}

