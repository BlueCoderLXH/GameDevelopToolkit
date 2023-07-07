//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

struct FDelauneyTriangle {
    int32 v0, v1, v2;
};

class DUNGEONARCHITECTRUNTIME_API DelauneyTriangulator {
public:
    virtual ~DelauneyTriangulator() {
    }

    void AddPoint(const FVector2D& Point);
    void Triangulate();

    const TArray<FVector2D>& GetPoints() const { return Points; }
    const TArray<FDelauneyTriangle>& GetTriangles() const { return Triangles; }

protected:
    virtual void PerformTriangulation() = 0;

protected:
    TArray<FVector2D> Points;
    TArray<FDelauneyTriangle> Triangles;
};

