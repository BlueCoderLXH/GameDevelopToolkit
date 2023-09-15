//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/FloorPlan/Volumes/FloorVolumeQuery.h"

#include "Builders/FloorPlan/Volumes/FloorPlanRoomVolume.h"
#include "Core/Dungeon.h"
#include "Core/Utils/Rectangle.h"

/////////////////////// FSpatialCacheVolumeQuery /////////////////////////

FSpatialCacheVolumeQuery::FSpatialCacheVolumeQuery(const FVector& InGridSize, ADungeon* InDungeon) : GridSize(
    InGridSize) {
    DungeonInverserTransform = InDungeon ? InDungeon->GetActorTransform().Inverse() : FTransform::Identity;
}

void FSpatialCacheVolumeQuery::Initialize(const TArray<AFloorPlanRoomVolume*>& RoomVolumes) {
    for (const AFloorPlanRoomVolume* RoomVolume : RoomVolumes) {
        if (RoomVolume->Tags.Num() == 0) continue;

        FName VolumeId = RoomVolume->Tags[0];
        FRectangle VolumeBounds;
        RoomVolume->GetDungeonVolumeBounds(GridSize, VolumeBounds);

        for (int x = VolumeBounds.Location.X; x < VolumeBounds.Location.X + VolumeBounds.Size.X; x++) {
            for (int y = VolumeBounds.Location.Y; y < VolumeBounds.Location.Y + VolumeBounds.Size.Y; y++) {
                for (int z = VolumeBounds.Location.Z; z < VolumeBounds.Location.Z + VolumeBounds.Size.Z; z++) {
                    FVector WorldLocation = FVector(x + 0.5f, y + 0.5f, z + 0.25f) * GridSize;

                    if (RoomVolume->EncompassesPoint(WorldLocation)) {
                        // The points we are calculating now are with reference to the origin.  
                        // The volume, however is placed by the user relative to the dungeon transform
                        // So, test this by first bringing the origin space point to the dungeon space
                        uint32 Hash = GetHash(WorldLocation);

                        if (LocationToVolumeMap.Contains(Hash)) {
                            LocationToVolumeMap[Hash] = VolumeId;
                        }
                        else {
                            LocationToVolumeMap.Add(Hash, VolumeId);
                        }
                    }
                }
            }
        }
    }
}

FName FSpatialCacheVolumeQuery::GetVolumeIdAt(const FVector& WorldLocation) {
    uint32 Hash = GetHash(WorldLocation);
    return LocationToVolumeMap.Contains(Hash) ? LocationToVolumeMap[Hash] : FName();
}

uint32 FSpatialCacheVolumeQuery::GetHash(const FVector& WorldLocation) {
    FVector LocationOriginSpace = DungeonInverserTransform.TransformPosition(WorldLocation);
    LocationOriginSpace /= GridSize;
    int32 gx = FMath::FloorToInt(LocationOriginSpace.X);
    int32 gy = FMath::FloorToInt(LocationOriginSpace.Y);
    int32 gz = FMath::FloorToInt(LocationOriginSpace.Z);
    uint32 Hash = GetTypeHash(FIntVector(gx, gy, gz));
    return Hash;
}


////////////////////////////// FBurteForceVolumeQuery //////////////////////////////////

void FBurteForceVolumeQuery::Initialize(const TArray<AFloorPlanRoomVolume*>& InRoomVolumes) {
    this->RoomVolumes = InRoomVolumes;
}

FName FBurteForceVolumeQuery::GetVolumeIdAt(const FVector& WorldLocation) {
    for (AFloorPlanRoomVolume* RoomVolume : RoomVolumes) {
        if (RoomVolume->EncompassesPoint(WorldLocation)) {
            if (RoomVolume->Tags.Num() > 0) {
                FName VolumeId = RoomVolume->Tags[0];
                return VolumeId;
            }
        }
    }
    return FName(); // not found
}

