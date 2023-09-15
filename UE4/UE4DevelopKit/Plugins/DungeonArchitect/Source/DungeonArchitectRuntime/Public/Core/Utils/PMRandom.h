//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

/**
 * 
 */
class DUNGEONARCHITECTRUNTIME_API PMRandom {
public:
    PMRandom();
    void Init(int32 seed);
    float NextGaussianFloat();
    float NextGaussianFloat(float mean, float stdDev);
    float GetNextUniformFloat();

private:
    FRandomStream random;
};

