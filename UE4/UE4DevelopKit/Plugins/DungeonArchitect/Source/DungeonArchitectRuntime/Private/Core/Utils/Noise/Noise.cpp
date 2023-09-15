//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/Noise/Noise.h"


////////////////////////////// Value Noise //////////////////////////////

float FValueNoisePolicy::Sample(float x, float y, const FValueNoiseTable& NoiseTable) {
    FNoiseTableCell<float> Cell;
    NoiseTable.GetCell(x, y, Cell);

    float fx = FMath::Frac(x);
    float fy = FMath::Frac(y);
    return FMath::Lerp(
        FMath::Lerp(Cell.N00, Cell.N10, fx),
        FMath::Lerp(Cell.N01, Cell.N11, fx),
        fy);
}

float FValueNoisePolicy::GetRandom(const FRandomStream& InRandom) {
    return InRandom.FRand() * 2 - 1;
}

//////////////////////////// Gradient Noise /////////////////////////////

float FGradientNoisePolicy::Sample(float x, float y, const FGradientNoiseTable& NoiseTable) {
    FNoiseTableCell<FVector2D> Cell;
    NoiseTable.GetCell(x, y, Cell);

    float fx = FMath::Frac(x);
    float fy = FMath::Frac(y);
    FVector2D P(fx, fy);

    return FMath::Lerp(
        FMath::Lerp(
            FVector2D::DotProduct(Cell.N00, P - FVector2D(0, 0)),
            FVector2D::DotProduct(Cell.N10, P - FVector2D(1, 0)),
            fx),
        FMath::Lerp(
            FVector2D::DotProduct(Cell.N01, P - FVector2D(0, 1)),
            FVector2D::DotProduct(Cell.N11, P - FVector2D(1, 1)),
            fx),
        fy);
}

FVector2D FGradientNoisePolicy::GetRandom(const FRandomStream& InRandom) {
    float Angle = InRandom.FRand() * 2 * PI;

    return FVector2D(
        FMath::Cos(Angle),
        FMath::Sin(Angle));
}

//////////////////////////// Worley Noise /////////////////////////////

float FWorleyNoisePolicy::Sample(float x, float y, const FWorleyNoiseTable& NoiseTable) {
    int ix = FMath::FloorToInt(x);
    int iy = FMath::FloorToInt(y);
    float fx = FMath::Frac(x);
    float fy = FMath::Frac(y);
    if (fx < 0) fx += 1;
    if (fy < 0) fy += 1;

    int32 TableSize = NoiseTable.GetSize();
    FVector2D P(fx, fy);

    float BestDistSq = MAX_flt;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            // Calculate the neighbor coords
            int32 nx = GetNeighborIndex(ix + dx, TableSize);
            int32 ny = GetNeighborIndex(iy + dy, TableSize);

            FVector2D NeighborTableData = NoiseTable.GetTableData(nx, ny);
            FVector2D NeighborValue = NeighborTableData + FVector2D(dx, dy);
            float DistSq = (P - NeighborValue).SizeSquared();
            BestDistSq = FMath::Min(BestDistSq, DistSq);
        }
    }

    float Distance = FMath::Sqrt(BestDistSq);
    Distance = FMath::Clamp(Distance, 0.0f, 1.0f);

    // Convert from [0..1] to [-1..1]
    Distance = 1 - Distance;
    Distance = Distance * 2 - 1;
    return Distance;
}

FVector2D FWorleyNoisePolicy::GetRandom(const FRandomStream& InRandom) {
    float Angle = InRandom.FRand() * 2 * PI;

    return FVector2D(
        FMath::Cos(Angle),
        FMath::Sin(Angle));
}

