//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Rectangle.generated.h"

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FRectangle {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FIntVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FIntVector Size;

public:
    FRectangle();
    FRectangle(int32 x, int32 y, int32 width, int32 height);
    FRectangle(const FIntVector& InLocation, const FIntVector& InSize);

    FORCEINLINE int32 X() const { return Location.X; }
    FORCEINLINE int32 Y() const { return Location.Y; }
    FORCEINLINE int32 Width() const { return Size.X; }
    FORCEINLINE int32 Height() const { return Size.Y; }
    FORCEINLINE FIntVector Center() const { return Location + Size / 2; }
    FVector CenterF() const;

    void GetBorderPoints(TArray<FIntVector>& OutPoints) const;

    void ExpandBy(int32 InAmount);

    void Clip(const FRectangle& ClipBounds);

    FORCEINLINE bool Contains(const FRectangle& rect) const {
        return (X() <= rect.X()) &&
            ((rect.X() + rect.Width()) <= (X() + Width())) &&
            (Y() <= rect.Y()) &&
            ((rect.Y() + rect.Height()) <= (Y() + Height()));
    }

    FORCEINLINE bool Contains(const FIntVector& Point) const {
        return Contains(Point.X, Point.Y);
    }

    FORCEINLINE bool Contains(int x, int y) const {
        return X() <= x &&
            x < X() + Width() &&
            Y() <= y &&
            y < Y() + Height();
    }

    static FRectangle Intersect(const FRectangle& a, const FRectangle& b);

    bool IntersectsWith(const FRectangle& rect) const {
        return (rect.X() < X() + Width()) &&
            (X() < (rect.X() + rect.Width())) &&
            (rect.Y() < Y() + Height()) &&
            (Y() < rect.Y() + rect.Height());
    }

    FORCEINLINE bool operator==(const FRectangle& Other) const {
        return Location == Other.Location && Size == Other.Size;
    }

    FORCEINLINE uint32 GetTypeHash(const FRectangle& rect) {
        return FCrc::MemCrc_DEPRECATED(&rect, sizeof(FRectangle));
    }
};

