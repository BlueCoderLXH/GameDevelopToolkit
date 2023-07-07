//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/DungeonModelHelper.h"

#include "Core/Dungeon.h"
#include "Core/Utils/Rectangle.h"

UDungeonModelHelper::UDungeonModelHelper(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
}

const FName UDungeonModelHelper::GenericDungeonIdTag("Dungeon");


FVector UDungeonModelHelper::MakeVector(const FIntVector& In, float scale) {
    return FVector(In.X, In.Y, In.Z) * scale;
}

FIntVector UDungeonModelHelper::MakeIntVector(const FVector& In) {
    return FIntVector(
        FMath::RoundToInt(In.X),
        FMath::RoundToInt(In.Y),
        FMath::RoundToInt(In.Z)
    );
}

void UDungeonModelHelper::ExpandBounds(const FRectangle& Bounds, int32 Size, FRectangle& Result) {
    Result = Bounds;
    Result.Location.X -= Size;
    Result.Location.Y -= Size;
    //Result.Location.Z -= Size;

    Result.Size.X += Size * 2;
    Result.Size.Y += Size * 2;
    //Result.Size.Z += Size * 2;
}

void UDungeonModelHelper::GetCenterExtent(const FRectangle& Rectangle, FVector& Center, FVector& Extent) {
    Center = MakeVector(Rectangle.Location) + MakeVector(Rectangle.Size) / 2.0f;
    Extent = MakeVector(Rectangle.Size) / 2.0f;
}

void UDungeonModelHelper::GetRectBorderPoints(const FRectangle& Rectangle, TArray<FIntVector>& BorderPoints) {
    BorderPoints.Reset();
    int32 z = Rectangle.Location.Z;
    for (int32 x = Rectangle.Location.X; x < Rectangle.Location.X + Rectangle.Size.X; x++) {
        int32 y = Rectangle.Location.Y;
        BorderPoints.Add(FIntVector(x, y, z));
        y += Rectangle.Size.Y - 1;
        BorderPoints.Add(FIntVector(x, y, z));
    }

    for (int32 y = Rectangle.Location.Y + 1; y < Rectangle.Location.Y + Rectangle.Size.Y - 1; y++) {
        int32 x = Rectangle.Location.X;
        BorderPoints.Add(FIntVector(x, y, z));
        x += Rectangle.Size.X - 1;
        BorderPoints.Add(FIntVector(x, y, z));
    }
}

void UDungeonModelHelper::GetRectAreaPoints(const FRectangle& Rectangle, TArray<FIntVector>& BorderPoints) {
    BorderPoints.Reset();
    int32 z = Rectangle.Location.Z;
    for (int32 x = Rectangle.Location.X; x < Rectangle.Location.X + Rectangle.Size.X; x++) {
        for (int32 y = Rectangle.Location.Y; y < Rectangle.Location.Y + Rectangle.Size.Y; y++) {
            BorderPoints.Add(FIntVector(x, y, z));
        }
    }

}

FName UDungeonModelHelper::GetDungeonIdTag(ADungeon* Dungeon) {
    if (Dungeon) {
        const FString TagString = "Dungeon-" + Dungeon->Uid.ToString();
        return FName(*TagString);
    }
    return GenericDungeonIdTag;
}

bool UDungeonModelHelper::LineIntersectsRect(const FIntVector& p1, const FIntVector& p2, const FRectangle& r) {
    return LineIntersectsLine(p1, p2, FIntVector(r.X(), r.Y(), 0), FIntVector(r.X() + r.Width(), r.Y(), 0)) ||
        LineIntersectsLine(p1, p2, FIntVector(r.X() + r.Width(), r.Y(), 0),
                           FIntVector(r.X() + r.Width(), r.Y() + r.Height(), 0)) ||
        LineIntersectsLine(p1, p2, FIntVector(r.X() + r.Width(), r.Y() + r.Height(), 0),
                           FIntVector(r.X(), r.Y() + r.Height(), 0)) ||
        LineIntersectsLine(p1, p2, FIntVector(r.X(), r.Y() + r.Height(), 0), FIntVector(r.X(), r.Y(), 0)) ||
        (r.Contains(p1) && r.Contains(p2));
}

bool UDungeonModelHelper::LineIntersectsLine(const FIntVector& l1p1, const FIntVector& l1p2, const FIntVector& l2p1,
                                             const FIntVector& l2p2) {

    float q = (l1p1.Y - l2p1.Y) * (l2p2.X - l2p1.X) - (l1p1.X - l2p1.X) * (l2p2.Y - l2p1.Y);
    float d = (l1p2.X - l1p1.X) * (l2p2.Y - l2p1.Y) - (l1p2.Y - l1p1.Y) * (l2p2.X - l2p1.X);

    if (d == 0) {
        return false;
    }

    float r = q / d;

    q = (l1p1.Y - l2p1.Y) * (l1p2.X - l1p1.X) - (l1p1.X - l2p1.X) * (l1p2.Y - l1p1.Y);
    float s = q / d;

    if (r < 0 || r > 1 || s < 0 || s > 1) {
        return false;
    }

    return true;
}

bool UDungeonModelHelper::GetNodeId(const FName& DungeonTag, AActor* Actor, FName& OutNodeId) {
    if (!Actor) return false;
    if (!Actor->IsValidLowLevel()) return false;
    if (!Actor->ActorHasTag(DungeonTag)) return false;
    if (Actor->IsPendingKill()) return false;

    // Find the Node tag
    for (const FName& Tag : Actor->Tags) {
        FString TagString = Tag.ToString();
        bool bNodeTag = TagString.StartsWith("NODE-");
        if (bNodeTag) {
            FString NodeIdStr = TagString.RightChop(5);
            OutNodeId = FName(*NodeIdStr);
            return true;
        }
    }
    return false;
}

