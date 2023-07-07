//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonConfig.h"
#include "GridDungeonConfig.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GridDungeonConfigLog, Log, All);

UENUM()
enum class EGridDungeonWallType : uint8 {
    WallsAsEdges UMETA(DisplayName = "Walls as Edges"),
    WallsAsTileBlocks UMETA(DisplayName = "Walls as Tile Blocks")
};

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridDungeonConfig : public UDungeonConfig {
    GENERATED_UCLASS_BODY()

public:
    /** 
      The number of cells to use while building the dungeon. 
      A larger number would create a bigger and more complex dungeon.   
      A number of 100-150 builds a medium to large sized dungeon. 
      Experiment with different ranges. You will not see these cells in the final result.
      */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 NumCells;

    /**
    This size is determined by the art asset used in the dungeon theme designed by the artist.
    For e.g., if we have a floor mesh that is 400x400.  The height of a floor is chosen to be 200 units as the stair mesh is 200 units high,
    then the value should be set to 400x400x200.   You should change this to the dimension of the modular asset your designer has created for the dungeon
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FVector GridCellSize;

    /**
      Minimum size of a generated cell. While generation, a cell is 
      either converted to a room, corridor or is discarded completely.  
      The Cell width / height is randomly chosen within this range
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MinCellSize;

    /**
      Maximum size of a generated cell. While generation, a cell is
      either converted to a room, corridor or is discarded completely.
      The Cell width / height is randomly chosen within this range
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MaxCellSize;

    /** 
      If a cell area exceeds past this limit, it is converted into a room. 
      After cells are promoted to rooms, all rooms are connected to each 
      other through corridors (either directly or indirectly. See spanning tree later)
      This field determines how "dense" your dungeon gets.  
      Reduce this number to get a densely populated dungeon with lots of rooms
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 RoomAreaThreshold;

    /** 
      The aspect ratio of the cells (width to height ratio).  
      Keeping this value near 0 would create square rooms.   
      Bringing this close to 1 would create elongated / stretched rooms 
      with a high width to height ratio
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float RoomAspectDelta;

    /** 
      Determines how many loops you would like to have in your dungeon.  
      A value near 0 will create fewer loops creating linear dungeons.   
      A value near 1 would create lots of loops, which would look unoriginal.  
      It's good to allow a few loops so a value close to zero (~0.2 should be good)
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float SpanningTreeLoopProbability;

    /**
      The generator would add stairs to make different areas of the dungeon accessible.
      However, we do not want too many stairs. For e.g., before adding a stair in a 
      particular elevated area, the generator would check if this area is already 
      accessible from a nearby stair. If so, it would not add it.   
      This tolerance parameter determines how far to look for an existing path
      before we can add a stair.   Play with this parameter if you see too
      many stairs close to each other, or too few
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float StairConnectionTollerance;

    /**
      Increase this value to remove nearby duplicate doors.  This value determines how many cell we 
      can move to reach the two connected cells of a door if the door was removed
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float DoorProximitySteps;

    /**
      weak this value to increase / reduce the height variations (and stairs) in your dungeon.
      A value close to 0 reduces the height variation and increases as you approach 1. 
      Increasing this value to a higher level might create dungeons with no place for
      proper stair placement since there is too much height variation.   
      A value of 0.2 to 0.4 seems good
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float HeightVariationProbability;

    /**
      The random number generator used in the dungeon generator does not use a uniform distribution. 
      Instead it uses a normal distribution to get higher frequency of lower values and fewer higher values
      (and hence fewer room cells and a lot more corridor cells). 
      Play with these parameters for different results
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float NormalMean;

    /**
      The random number generator used in the dungeon generator does not use a uniform distribution.
      Instead it uses a normal distribution to get higher frequency of lower values and fewer higher values
      (and hence fewer room cells and a lot more corridor cells).
      Play with these parameters for different results
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float NormalStd;

    /**
      The number of logical floor units the dungeon height can vary. This determines how high the dungeon's height
      can vary (e.g. max 2 floors high).   Set this value depending on the stair meshes you designer has created. 
      For e.g.,  if there are two stair meshes, one 200 units high (1 floor) and another 400 units high (2 floors), this value should be set to 2
      If you do not want any stairs / height variations, set this value to 0
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MaxAllowedStairHeight;

    /** 
      The width of the lane connecting the rooms.   When rooms are to be connected, a "lane" is created to connect them together. 
      Any cell colliding with this lane is converted to a corridor.  Else, it is converted to a cell of type "corridor padding"
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 LaneWidth;

    /**
      This allows you to cluster the dungeon into different groups and apply different themes on each groups.  This helps add variation to the level
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool bEnableClusteredTheming;

    /**
      While clustering corridors together, should the height variations be taken into account,
      If true, will group nearby cells with different heights, provided a stair connects to them.
      This will create clusters of larger sections of corridors together
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool bClusterWithHeightVariation;

    /** [Advanced] Ignored */
    UPROPERTY() //(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 FloorHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon, meta = (Experimental))
    EGridDungeonWallType WallLayoutType = EGridDungeonWallType::WallsAsEdges;


    /** [Advanced]  Keep between 10-15 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float InitialRoomRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon, meta = (Experimental))
    bool bFastCellDistribution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon, meta = (EditCondition = bFastCellDistribution,
        Experimental))
    int32 DungeonWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon, meta = (EditCondition = bFastCellDistribution,
        Experimental))
    int32 DungeonLength;

};

