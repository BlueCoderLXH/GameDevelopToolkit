//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SimpleCity/Utils/SimpleCityRoadBeautifier.h"

#include "Builders/SimpleCity/SimpleCityModel.h"
#include "Builders/SimpleCity/Utils/SimpleCityConstants.h"

#define CITY_INDEX(x, y) ((y) * CityModel->CityWidth + (x))

FString FSimpleCityRoadBeautifier::GetRoadMarkerName(int x, int y, USimpleCityModel* CityModel, float& OutAngle) {
    OutAngle = 0;

    int ConfigRoadX[] = {
        1, 0,
        0, 1,
        -1, 0,
        0, -1
    };

    int ConfigRoadT[] = {
        1, 0,
        0, 1,
        -1, 0
    };

    int ConfigRoadCorner[] = {
        1, 0,
        0, 1
    };

    int ConfigRoadS[] = {
        1, 0,
        -1, 0
    };
    int ConfigRoadEnd[] = {
        1, 0,
    };

    if (MatchesConfig(x, y, CityModel, OutAngle, ConfigRoadX, 8)) return SimpleCityDungeonConstants::MarkerRoadX;
    if (MatchesConfig(x, y, CityModel, OutAngle, ConfigRoadT, 6)) return SimpleCityDungeonConstants::MarkerRoadT;
    if (MatchesConfig(x, y, CityModel, OutAngle, ConfigRoadCorner, 4)) return
        SimpleCityDungeonConstants::MarkerRoadCorner;
    if (MatchesConfig(x, y, CityModel, OutAngle, ConfigRoadS, 4)) return SimpleCityDungeonConstants::MarkerRoadS;
    if (MatchesConfig(x, y, CityModel, OutAngle, ConfigRoadEnd, 2)) return SimpleCityDungeonConstants::MarkerRoadEnd;

    return SimpleCityDungeonConstants::MarkerRoad;
}

bool FSimpleCityRoadBeautifier::MatchesConfig(int x, int y, USimpleCityModel* CityModel, float& OutAngle,
                                              int* Neighbors, int32 NeighborCount) {
    OutAngle = 0;
    for (int32 da = 0; da < 4; da++) {
        bool rejected = false;
        for (int i = 0; i + 1 < NeighborCount; i += 2) {
            int dirX = Neighbors[i];
            int dirY = Neighbors[i + 1];
            FVector direction(dirX, dirY, 0);
            direction = FQuat::MakeFromEuler(FVector(0, 0, da * 90)) * direction;

            int32 nx = FMath::RoundToInt(x + direction.X);
            int32 nz = FMath::RoundToInt(y + direction.Y);

            if (!ContainsRoad(nx, nz, CityModel)) {
                rejected = true;
                break;
            }
        }
        if (!rejected) {
            OutAngle = da * 90;
            return true;
        }
    }
    return false; // Not found
}

bool FSimpleCityRoadBeautifier::ContainsRoad(int x, int y, USimpleCityModel* CityModel) {
    int32 lx = CityModel->CityWidth;
    int32 ly = CityModel->CityLength;
    if (x < 0 || y < 0 || x >= lx || y >= ly) {
        return false;
    }
    return CityModel->Cells[CITY_INDEX(x, y)].CellType == ESimpleCityCellType::Road;
}

#undef CITY_INDEX

