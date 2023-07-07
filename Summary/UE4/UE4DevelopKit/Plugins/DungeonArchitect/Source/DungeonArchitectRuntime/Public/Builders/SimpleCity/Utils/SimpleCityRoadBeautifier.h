//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class USimpleCityModel;

class DUNGEONARCHITECTRUNTIME_API FSimpleCityRoadBeautifier {
public:
    static FString GetRoadMarkerName(int x, int z, USimpleCityModel* CityModel, float& OutAngle);
    static bool MatchesConfig(int x, int z, USimpleCityModel* CityModel, float& OutAngle, int* Neighbors,
                              int32 NeighborCount);
    static bool ContainsRoad(int x, int z, USimpleCityModel* CityModel);

};

