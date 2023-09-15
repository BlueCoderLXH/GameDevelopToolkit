//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class UGrammarRuleScriptGraphNode;

class DUNGEONARCHITECTRUNTIME_API FMathUtils {
public:
    static FORCEINLINE FVector ToVector(const FIntVector& value) {
        return FVector(value.X, value.Y, value.Z);
    }
    
    static FIntVector ToIntVector(const FVector& value, bool bRound) {
        if (bRound) {
            return FIntVector(
                FMath::RoundToInt(value.X),
                FMath::RoundToInt(value.Y),
                FMath::RoundToInt(value.Z));
        }
        return FIntVector(
            FMath::FloorToInt(value.X),
            FMath::FloorToInt(value.Y),
            FMath::FloorToInt(value.Z));
    }

    
    static FIntPoint ToIntPoint(const FVector2D& value, bool bRound = false) {
        if (bRound) {
            return FIntPoint(
                FMath::RoundToInt(value.X),
                FMath::RoundToInt(value.Y));
        }
        return FIntPoint(
            FMath::FloorToInt(value.X),
            FMath::FloorToInt(value.Y));
    }

    static FORCEINLINE FVector2D GetRandomDirection2D(FRandomStream& Random) {
        float Angle = Random.FRand() * PI * 2;
        return FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
    }

    static TArray<int32> GetShuffledIndices(int32 Count, const FRandomStream& Random);

    template<typename T>
    static void Swap(T& A, T& B) {
        T Temp = A;
        A = B;
        B = Temp;
    }
    
    template <typename T>
    static T GetRandomItem(TArray<T>& Array, FRandomStream& Random) {
        return Array[Random.RandRange(0, Array.Num() - 1)];
    }

    template <typename T>
    static void Shuffle(TArray<T>& Array, const FRandomStream& Random) {
        int32 Count = Array.Num();
        for (int i = 0; i < Count; i++) {
            int32 j = Random.RandRange(0, Count - 1);
            T Temp = Array[i];
            Array[i] = Array[j];
            Array[j] = Temp;
        }
    }
};

// Blur algorithm from: http://blog.ivank.net/fastest-gaussian-blur.html (MIT License)
class DUNGEONARCHITECTRUNTIME_API BlurUtils {
public:

    static void boxBlurH_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r);
    static void boxBlurT_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r);
    static void boxBlur_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r);
    static TArray<int32> boxesForGauss(float sigma, float n); // standard deviation, number of boxes
    static void gaussBlur_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r);
    FORCEINLINE static float BlurRound(float Value) {
        return Value;
    }
};

class DUNGEONARCHITECTRUNTIME_API FColorUtils {
public:
    static FLinearColor BrightenColor(const FLinearColor& InColor, float SaturationMultiplier,
                                      float BrightnessMultiplier);
};

