//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/Rectangle.h"

#include "Core/Utils/MathUtils.h"

FRectangle::FRectangle() {
    Location = FIntVector(0, 0, 0);
    Size = FIntVector(0, 0, 0);
}

FRectangle::FRectangle(int32 x, int32 y, int32 width, int32 height) {
    Location = FIntVector(x, y, 0);
    Size = FIntVector(width, height, 0);
}

FRectangle::FRectangle(const FIntVector& InLocation, const FIntVector& InSize): Location(InLocation)
                                                                                , Size(InSize) {
}

FVector FRectangle::CenterF() const {
    return FMathUtils::ToVector(Location) + FMathUtils::ToVector(Size) * 0.5f;
}

void FRectangle::GetBorderPoints(TArray<FIntVector>& OutPoints) const {
    for (int dx = 0; dx < Size.X; dx++) {
        int x = Location.X + dx;
        int y = Location.Y;
        int z = Location.Z;
        FIntVector point = FIntVector(x, y, z);
        OutPoints.Add(point);

        if (Size.Y > 1) {
            y = Location.Y + Size.Y - 1;
            point = FIntVector(x, y, z);
            OutPoints.Add(point);
        }
    }

    for (int dy = 1; dy < Size.Y - 1; dy++) {
        int x = Location.X;
        int y = Location.Y + dy;
        int z = Location.Z;
        FIntVector point = FIntVector(x, y, z);
        OutPoints.Add(point);

        if (Size.X > 1) {
            x = Location.X + Size.X - 1;
            point = FIntVector(x, y, z);
            OutPoints.Add(point);
        }
    }
}

void FRectangle::ExpandBy(int32 InAmount) {
    Location.X -= InAmount;
    Location.Y -= InAmount;

    Size.X += InAmount * 2;
    Size.Y += InAmount * 2;
}

void FRectangle::Clip(const FRectangle& ClipBounds) {
    int32 X0 = Location.X;
    int32 X1 = Location.X + Size.X;
    int32 Y0 = Location.Y;
    int32 Y1 = Location.Y + Size.Y;
    int32 Z0 = Location.Z;
    int32 Z1 = Location.Z + Size.Z;

    int32 CX0 = ClipBounds.Location.X;
    int32 CX1 = ClipBounds.Location.X + ClipBounds.Size.X;
    int32 CY0 = ClipBounds.Location.Y;
    int32 CY1 = ClipBounds.Location.Y + ClipBounds.Size.Y;
    int32 CZ0 = ClipBounds.Location.Z;
    int32 CZ1 = ClipBounds.Location.Z + ClipBounds.Size.Z;

    X0 = FMath::Clamp(X0, CX0, CX1);
    X1 = FMath::Clamp(X1, CX0, CX1);
    Y0 = FMath::Clamp(Y0, CY0, CY1);
    Y1 = FMath::Clamp(Y1, CY0, CY1);
    Z0 = FMath::Clamp(Z0, CZ0, CZ1);
    Z1 = FMath::Clamp(Z1, CZ0, CZ1);

    Location = FIntVector(X0, Y0, Z0);
    Size = FIntVector(X1 - X0, Y1 - Y0, Z1 - Z0);
}

FRectangle FRectangle::Intersect(const FRectangle& a, const FRectangle& b) {
    int x1 = FMath::Max(a.X(), b.X());
    int x2 = FMath::Min(a.X() + a.Width(), b.X() + b.Width());
    int y1 = FMath::Max(a.Y(), b.Y());
    int y2 = FMath::Min(a.Y() + a.Height(), b.Y() + b.Height());

    if (x2 >= x1 && y2 >= y1) {
        return FRectangle(x1, y1, x2 - x1, y2 - y1);
    }
    return FRectangle();
}

