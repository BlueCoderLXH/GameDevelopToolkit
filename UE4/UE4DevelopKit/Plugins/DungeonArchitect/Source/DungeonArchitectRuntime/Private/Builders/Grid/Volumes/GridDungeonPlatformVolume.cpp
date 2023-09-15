//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/Volumes/GridDungeonPlatformVolume.h"

#include "Components/BrushComponent.h"
#include "Engine/CollisionProfile.h"

AGridDungeonPlatformVolume::AGridDungeonPlatformVolume(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer),
      CellType(FCellType::Corridor) {
    UBrushComponent* BrushComp = GetBrushComponent();
    if (BrushComp) {
        BrushComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
        BrushComp->SetGenerateOverlapEvents(false);
    }
}

