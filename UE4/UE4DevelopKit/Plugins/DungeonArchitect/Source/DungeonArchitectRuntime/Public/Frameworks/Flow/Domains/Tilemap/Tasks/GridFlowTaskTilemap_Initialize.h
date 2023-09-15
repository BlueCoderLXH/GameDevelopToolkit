//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/Array2D.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemapBase.h"
#include "GridFlowTaskTilemap_Initialize.generated.h"

class UGridFlowTilemap;
class UGridFlowAbstractGraph;
class UFlowAbstractNode;
enum class EGridFlowAbstractNodeRoomType : uint8;
template<typename TNode> class TFlowAbstractGraphQuery; 
typedef TFlowAbstractGraphQuery<UFlowAbstractNode> FFlowAbstractGraphQuery;

struct FGridFlowTilemapNodeInfo {
    FGridFlowTilemapNodeInfo()
        : x0(0)
          , x1(0)
          , y0(0)
          , y1(0)
          , midX(0)
          , midY(0) {
    }

    FGridFlowTilemapNodeInfo(float InX0, float InY0, float InX1, float InY1) {
        x0 = InX0;
        x1 = InX1;
        y0 = InY0;
        y1 = InY1;

        midX = (x0 + x1) * 0.5f;
        midY = (y0 + y1) * 0.5f;
    }

    float x0;
    float x1;
    float y0;
    float y1;

    float midX;
    float midY;

    FGuid AbstractNodeId;
};

typedef TDAArray2D<FGridFlowTilemapNodeInfo> FGridFlowTilemapNodes;

struct FGFCaveCellBuildTile {
    FIntPoint TileCoord;
    bool bValid = false;
    bool bRockTile = false;
};

typedef TDAArray2D<FGFCaveCellBuildTile> FGFCaveCellBuildTiles;


UENUM()
enum class EGridFlowTilemapWallGenerationMethod : uint8
{
    WallAsTiles UMETA(DisplayName = "Wall as Tiles"),
    WallAsEdges UMETA(DisplayName = "Wall as Edges")
};

UCLASS(Meta = (TilemapTask, Title = "Create Tilemap", Tooltip = "Create Tilemap", MenuPriority = 2100))
class DUNGEONARCHITECTRUNTIME_API UGridFlowTaskTilemap_Initialize : public UGridFlowTaskTilemapBase {
    GENERATED_BODY()

public:
    /**
        The nodes in the layout graph are converted into rooms, corridors or cave chunks. 
        This determines how many tiles in the tilemap are used to generate a room
        Increase this number to get more tiles in the room (also also increase the size of the tilemap)

        Variable Name: TilemapSizePerNode
    */
    UPROPERTY(EditAnywhere, Category = "Layout Settings")
    FIntPoint TilemapSizePerNode = FIntPoint(10, 10);

    /**
        The room walls are move around to give an uneven look.  Bring this value down to get more evenly spaced square rooms

        Variable Name: PerturbAmount
    */
    UPROPERTY(EditAnywhere, Category = "Layout Settings")
    FIntPoint PerturbAmount = FIntPoint(3, 3);

    /**
        The width in tile cells for the generated corridors in the tilemap

        Variable Name: CorridorLaneWidth
    */
    UPROPERTY(EditAnywhere, Category = "Layout Settings")
    int32 CorridorLaneWidth = 2;

    /**
        The tilemap is cropped to the layout boundaries.  
        Add padding on the tilemap around the layout boundaries

        Variable Name: LayoutPadding
    */
    UPROPERTY(EditAnywhere, Category = "Layout Settings")
    int32 LayoutPadding = 0;

    /**
        After the dungeon has been built, the tilemap is cropped to fit the dungeon layout
        Disable this if you rely on static room positions (like blending static art asset to enter the dungeon)

        Variable Name: bCropTilemapToLayout
    */
    UPROPERTY(EditAnywhere, Category = "Layout Settings")
    bool bCropTilemapToLayout = false;

    /**
        Configure walls to be thick (that take up an entire tile) or thin that are place don the edge of a tile.
        This depends on your art asset used in the theme file. Set accordingly
     **/
    UPROPERTY(EditAnywhere, Category = "Layout Settings")
    EGridFlowTilemapWallGenerationMethod WallGenerationMethod = EGridFlowTilemapWallGenerationMethod::WallAsTiles;
    
    /**
        Control the thickness of the generate caves

        Variable Name: CaveThickness
    */
    UPROPERTY(EditAnywhere, Category = "Cave Settings")
    float CaveThickness = 2.25f;

    /**
        Cave Generation Finite Automata setting. The number of rock neighbors for a 
        tile to turn into a rock

        Variable Name: CaveAutomataNeighbors
    */
    UPROPERTY(EditAnywhere, Category = "Cave Settings")
    int32 CaveAutomataNeighbors = 5;

    /**
        Cave Generation Finite Automata setting. The number of iterations to run

        Variable Name: CaveAutomataIterations
    */
    UPROPERTY(EditAnywhere, Category = "Cave Settings")
    int32 CaveAutomataIterations = 4;

    /**
        When a layout node is selected, the relevant tiles in the tilemap light up. 
        This setting controls the color.  This is only for preview purpose

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Preview Settings")
    float RoomColorSaturation = 0.3f;

    /**
        When a layout node is selected, the relevant tiles in the tilemap light up.
        This setting controls the color.  This is only for preview purpose

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Preview Settings")
    float RoomColorBrightness = 1.5f;

    /**
        Debug: Shows the layout tiles in red

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugLayoutTiles = false;

public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;

private:
    UGridFlowTilemap* BuildTilemap(UGridFlowAbstractGraph* Graph, const FRandomStream& Random);

    void PerturbRoomSizes(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                          const FFlowAbstractGraphQuery& GraphQuery, const FRandomStream& Random);
    void FixCorridorSizes(FGridFlowTilemapNodes& TileNodes, const FFlowAbstractGraphQuery& GraphQuery) const;
    void InitRasterization(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                           const FFlowAbstractGraphQuery& GraphQuery) const;
    void RasterizeRoomCorridors(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                const FFlowAbstractGraphQuery& GraphQuery) const;
    void GenerateMainPath(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                          const FFlowAbstractGraphQuery& GraphQuery);
    bool ShouldBlockCaveBoundary(UGridFlowAbstractGraph* Graph, UFlowAbstractNode* CaveNode, int dx, int dy) const;
    FIntPoint NodeCoordToTileCoord(const FVector& NodeCoord) const;
    void CalculateDistanceFromMainPath(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                       const FFlowAbstractGraphQuery& GraphQuery,
                                       TArray<EGridFlowAbstractNodeRoomType> AllowedRoomTypes) const;
    void RasterizeBaseCaveBlocks(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                 const FFlowAbstractGraphQuery& GraphQuery) const;
    void BuildCaves(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                    const FFlowAbstractGraphQuery& GraphQuery, const FRandomStream& Random);
    static void GenerateCaveMap(FGFCaveCellBuildTiles& CaveMap, FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                const FFlowAbstractGraphQuery& GraphQuery);
    void BuildCaveStep_BuildRocks(FGFCaveCellBuildTiles& CaveMap, UGridFlowTilemap* Tilemap,
                                  const FRandomStream& Random) const;
    void BuildCaveStep_SimulateGrowth(FGFCaveCellBuildTiles& CaveMap, UGridFlowTilemap* Tilemap,
                                      const FRandomStream& Random) const;
    void BuildCaveStep_Cleanup(FGFCaveCellBuildTiles& CaveMap, FGridFlowTilemapNodes& TileNodes,
                               UGridFlowTilemap* Tilemap, const FFlowAbstractGraphQuery& GraphQuery) const;
    static void BuildCaveStep_UpdateEdges(FGFCaveCellBuildTiles& CaveMap, UGridFlowTilemap* Tilemap);
    
private:
    FIntPoint LayoutGridSize;
    void BuildDoors(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                    const FFlowAbstractGraphQuery& GraphQuery) const;
    UGridFlowTilemap* ApplyCropAndPadding(UGridFlowTilemap* Tilemap) const;
    void CalculateDistanceFromMainPathOnEmptyArea(UGridFlowTilemap* Tilemap) const;
};

