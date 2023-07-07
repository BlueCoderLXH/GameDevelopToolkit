//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/MathUtils.h"

/*
template<typename T>
void IterateRandom(const FRandomStream& Random, const TArray<T*>& InList, TFunction<void(const T*)> Body) {
    TArray<int32> Indices = FMathUtils::GetShuffledIndices(InList.Num(), Random);
    for (int32 Index : Indices) {
        const T* Item = InList[Index];
        Body(Item);
    }
}
*/

class FRandomIterator {
public:
    FRandomIterator(const FRandomStream* InRandom) : Random(InRandom) {}

    template<typename T>
    void Iterate(const TArray<T>& InList, TFunction<void(const T&)> Body) {
        TArray<int32> Indices = FMathUtils::GetShuffledIndices(InList.Num(), *Random);
        for (int32 Index : Indices) {
            const T& Item = InList[Index];
            Body(Item);
        }
    }

private:
    const FRandomStream* Random;
}; 

