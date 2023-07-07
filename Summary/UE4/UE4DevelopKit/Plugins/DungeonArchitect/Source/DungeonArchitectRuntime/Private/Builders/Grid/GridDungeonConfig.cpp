//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/GridDungeonConfig.h"


DEFINE_LOG_CATEGORY(GridDungeonConfigLog);

UGridDungeonConfig::UGridDungeonConfig(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
      , NumCells(100)
      , GridCellSize(FVector(400, 400, 200))
      , MinCellSize(2)
      , MaxCellSize(5)
      , RoomAreaThreshold(15)
      , RoomAspectDelta(0.4f)
      , SpanningTreeLoopProbability(0.15f)
      , StairConnectionTollerance(6)
      , DoorProximitySteps(6)
      , HeightVariationProbability(0.2f)
      , NormalMean(0)
      , NormalStd(0.3f)
      , MaxAllowedStairHeight(2)
      , LaneWidth(2)
      , bEnableClusteredTheming(false)
      , bClusterWithHeightVariation(true)
      , FloorHeight(0)
      , InitialRoomRadius(15)
      , DungeonWidth(10000)
      , DungeonLength(10000) {

}

