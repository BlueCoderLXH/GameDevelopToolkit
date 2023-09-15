//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class DUNGEONARCHITECTRUNTIME_API FSpatialConstraintUtils {
public:
    template <typename CellDataType>
    static TArray<CellDataType> RotateNeighborConfig3x3(const TArray<CellDataType>& Neighbors) {
        const int SrcIndex[] = {
            0, 1, 2,
            3, 4, 5,
            6, 7, 8
        };
        const int DstIndex[] = {
            6, 3, 0,
            7, 4, 1,
            8, 5, 2
        };

        TArray<CellDataType> Result;
        Result.SetNumUninitialized(9);
        for (int i = 0; i < 9; i++) {
            Result[DstIndex[i]] = Neighbors[SrcIndex[i]];
        }
        return Result;
    }

    template <typename CellDataType>
    static TArray<CellDataType> RotateNeighborConfig2x2(const TArray<CellDataType>& Neighbors) {
        const int SrcIndex[] = {
            0, 1,
            2, 3
        };
        const int DstIndex[] = {
            2, 0,
            3, 1
        };

        TArray<CellDataType> Result;
        Result.SetNumUninitialized(4);
        for (int i = 0; i < 4; i++) {
            Result[DstIndex[i]] = Neighbors[SrcIndex[i]];
        }
        return Result;
    }


};

