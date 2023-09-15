//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Volumes/DungeonVolume.h"

#include "Core/Utils/Rectangle.h"

#include "Engine/World.h"

const int32 VOLUME_UNIT_CM_SCALE = 100;

ADungeonVolume::ADungeonVolume(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void ADungeonVolume::GetDungeonVolumeBounds(const FVector& GridCellSize, FRectangle& OutBounds) const {
    FVector BoxOrigin, BoxExtent;
    GetActorBounds(false, BoxOrigin, BoxExtent);

    FVector Start = BoxOrigin - BoxExtent;
    FVector End = BoxOrigin + BoxExtent;
    FVector Size = End - Start;

    FVector GStart = Start / GridCellSize;
    OutBounds.Location.X = FMath::RoundToInt(GStart.X);
    OutBounds.Location.Y = FMath::RoundToInt(GStart.Y);
    OutBounds.Location.Z = FMath::RoundToInt(GStart.Z);

    FVector GSize = Size / GridCellSize;
    OutBounds.Size.X = FMath::RoundToInt(GSize.X);
    OutBounds.Size.Y = FMath::RoundToInt(GSize.Y);
    OutBounds.Size.Z = FMath::RoundToInt(GSize.Z);
}

void ADungeonVolume::Tick(float DeltaSeconds) {
    bool bEditorMode = false;
    UWorld* World = GetWorld();
    if (World) {
        if (World->WorldType == EWorldType::Editor) {
            bEditorMode = true;
        }
    }
}

#if WITH_EDITOR
void ADungeonVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
    Super::PostEditChangeProperty(PropertyChangedEvent);
    RebuildDungeon();
}

void ADungeonVolume::PostEditMove(bool bFinished) {
    Super::PostEditMove(bFinished);
    RebuildDungeon();
}
#endif

void ADungeonVolume::RebuildDungeon() {
    if (RealtimeUpdate && Dungeon) {
        Dungeon->BuildDungeon();
    }
}

