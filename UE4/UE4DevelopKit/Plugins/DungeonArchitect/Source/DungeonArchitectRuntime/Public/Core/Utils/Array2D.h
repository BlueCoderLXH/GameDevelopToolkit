//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

template <typename T>
class TDAArray2D {
public:
    FORCEINLINE T& operator[](const FIntPoint& Coord) {
        return Get(Coord.X, Coord.Y);
    }

    FORCEINLINE const T& operator[](const FIntPoint& Coord) const {
        return Get(Coord.X, Coord.Y);
    }

    FORCEINLINE T& operator[](const FIntVector& Coord) {
        return Get(Coord.X, Coord.Y);
    }
    
    FORCEINLINE const T& operator[](const FIntVector& Coord) const {
        return Get(Coord.X, Coord.Y);
    }
    
    FORCEINLINE T& Get(int32 X, int32 Y) {
        return Array[INDEX(X, Y)];
    }

    FORCEINLINE const T& Get(int32 X, int32 Y) const {
        return Array[INDEX(X, Y)];
    }

    FORCEINLINE T* GetSafe(int32 X, int32 Y) {
        return (X >= 0 && Y >= 0 && X < Width && Y < Height)
                   ? &Array[INDEX(X, Y)]
                   : nullptr;
    }

    FORCEINLINE const T* GetSafe(int32 X, int32 Y) const {
        return (X >= 0 && Y >= 0 && X < Width && Y < Height)
                   ? &Array[INDEX(X, Y)]
                   : nullptr;
    }

    FORCEINLINE void Set(int32 X, int32 Y, T& Value) {
        Array[INDEX(X, Y)] = Value;
    }

    FORCEINLINE TArray<T>& GetCells() { return Array; }
    FORCEINLINE const TArray<T>& GetCells() const { return Array; }
    FORCEINLINE int32 GetWidth() const { return Width; }
    FORCEINLINE int32 GetHeight() const { return Height; }

    void InitializeArray2D(int32 InWidth, int32 InHeight) {
        Width = InWidth;
        Height = InHeight;
        Array.Reset();
        Array.AddDefaulted(Width * Height);
    }

protected:
    FORCEINLINE int32 INDEX(int32 X, int32 Y) const { return Y * Width + X; }

protected:
    TArray<T> Array;
    int32 Width = 0;
    int32 Height = 0;
};

