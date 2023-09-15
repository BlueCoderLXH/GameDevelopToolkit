//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/Array2D.h"
#include "GridFlowTilemap.generated.h"

UENUM(BlueprintType)
enum class EGridFlowTilemapCellType : uint8 {
    Empty,
    Floor,
    Wall,
    Door,
    Custom
};

UENUM(BlueprintType)
enum class EGridFlowTilemapEdgeType : uint8 {
    Empty,
    Wall,
    Fence,
    Door
};

UENUM(BlueprintType)
enum class EGridFlowTilemapCellCategory : uint8 {
    Layout,
    Biome,
    Elevation
};

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FGridFlowTilemapCustomCellInfo {
    GENERATED_BODY()

    UPROPERTY()
    FString MarkerName;

    UPROPERTY()
    FLinearColor DefaultColor = FLinearColor::White;
};

UENUM(BlueprintType)
enum class EGridFlowTilemapCellOverlayMergeWallOverlayRule : uint8 {
    KeepWallAndOverlay,
    KeepWallRemoveOverlay,
    KeepOverlayRemoveWall
};


USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FGridFlowTilemapCellOverlayMergeConfig {
    GENERATED_BODY()

    /**
        Minimum height of the tile for the merge to happen

        Variable Name: MinHeight
    */
    UPROPERTY(EditAnywhere, Category = "Merge Config")
    float MinHeight = 0;

    /**
        Maximum height of the tile for the merge to happen

        Variable Name: MaxHeight
    */
    UPROPERTY(EditAnywhere, Category = "Merge Config")
    float MaxHeight = 3;

    /**
        Control how the walls are treated when overlays are placed in wall tiles

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Merge Config")
    EGridFlowTilemapCellOverlayMergeWallOverlayRule WallOverlayRule =
        EGridFlowTilemapCellOverlayMergeWallOverlayRule::KeepWallAndOverlay;

    /**
        The logical height offset on the marker for tiles inside the dungeon layout

        Variable Name: MarkerHeightOffsetForLayoutTiles
    */
    UPROPERTY(EditAnywhere, Category = "Merge Config")
    float MarkerHeightOffsetForLayoutTiles = 0;

    /**
        The logical height offset on the marker for tiles outside the dungeon layout

        Variable Name: MarkerHeightOffsetForNonLayoutTiles
    */
    UPROPERTY(EditAnywhere, Category = "Merge Config")
    float MarkerHeightOffsetForNonLayoutTiles = 0;

    /**
        Should the elevation marker be removed when this overlay is placed on an elevation tile?

        Variable Name: RemoveElevationMarker
    */
    UPROPERTY(EditAnywhere, Category = "Merge Config")
    bool RemoveElevationMarker = false;
};


USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FGridFlowTilemapCoord {
    GENERATED_BODY()
    
    FGridFlowTilemapCoord() {}
    explicit FGridFlowTilemapCoord(const FIntPoint& InCoord)
        : Coord(InCoord)
        , bIsEdgeCoord(false)
    {}
    explicit FGridFlowTilemapCoord(const FIntPoint& InCoord, bool bInHorizontalEdge)
        : Coord(InCoord)
        , bIsEdgeCoord(true)
        , bHorizontalEdge(bInHorizontalEdge)
    {}
    
    /** The coordinate of the tile in the tilemap */
    UPROPERTY()
    FIntPoint Coord = FIntPoint::ZeroValue;

    /** Indicates the horizontal or the vertical edge on the tile. Ignored if we are referencing a tile and not an edge */
    UPROPERTY()
    bool bIsEdgeCoord = false;

    /** Indicates the horizontal or the vertical edge on the tile. Ignored if we are referencing a tile and not an edge */
    UPROPERTY()
    bool bHorizontalEdge = true;
    
};

FORCEINLINE uint32 GetTypeHash(const FGridFlowTilemapCoord& Coord) {
    return GetTypeHash(FIntVector4(Coord.Coord.X, Coord.Coord.Y,
        Coord.bIsEdgeCoord ? 0 : 1,
        Coord.bHorizontalEdge ? 0 : 1));
}
FORCEINLINE bool operator==(const FGridFlowTilemapCoord& A, const FGridFlowTilemapCoord& B) {
    return A.Coord == B.Coord
    && A.bIsEdgeCoord == B.bIsEdgeCoord
    && A.bHorizontalEdge == B.bHorizontalEdge;
}

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FGridFlowTilemapEdge {
    GENERATED_BODY()
    
    UPROPERTY()
    EGridFlowTilemapEdgeType EdgeType = EGridFlowTilemapEdgeType::Empty;
    
    UPROPERTY()
    FGuid ItemId;
    
    UPROPERTY()
    float HeightCoord = 0;
    
    UPROPERTY()
    float MarkerAngle = 0;
    
    UPROPERTY()
    bool bHasItem = false;
    
    UPROPERTY()
    FGridFlowTilemapCoord EdgeCoord;
};

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FGridFlowTilemapCellOverlay {
    GENERATED_BODY()

    UPROPERTY()
    bool bEnabled = false;

    UPROPERTY()
    FString MarkerName;

    UPROPERTY()
    FLinearColor Color = FLinearColor::White;

    UPROPERTY()
    float NoiseValue = 0;

    UPROPERTY()
    bool bTileBlockingOverlay = true;

    UPROPERTY()
    FGridFlowTilemapCellOverlayMergeConfig MergeConfig;
};

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FGridFlowTilemapCell {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    EGridFlowTilemapCellType CellType = EGridFlowTilemapCellType::Empty;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    FGridFlowTilemapCustomCellInfo CustomCellInfo;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    bool bHasItem = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    FGuid ItemId;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    TArray<FString> Tags;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    bool bHasOverlay = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    FGridFlowTilemapCellOverlay Overlay;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    FVector ChunkCoord = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    FIntPoint TileCoord = FIntPoint::ZeroValue;

    UPROPERTY()
    bool bUseCustomColor = false;

    UPROPERTY()
    FLinearColor CustomColor = FLinearColor::White;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    bool bMainPath = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    bool bLayoutCell = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    bool bUnreachable = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    int DistanceFromMainPath = MAX_int32;

    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    float Height = 0;

};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FGridFlowTilemapCellDoorInfo {
    GENERATED_BODY()

    UPROPERTY()
    bool bLocked = false;

    UPROPERTY()
    bool bOneWay = false;

    UPROPERTY()
    FIntVector ChunkA;

    UPROPERTY()
    FIntVector ChunkB;

    UPROPERTY()
    float Angle = 0;
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FGridFlowTilemapCellWallInfo {
    GENERATED_BODY()

    UPROPERTY()
    TArray<FIntPoint> OwningTiles;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridFlowTilemap : public UObject {
    GENERATED_BODY()

public:
    void Initialize(int32 InWidth, int32 InHeight);

    FORCEINLINE FGridFlowTilemapCell& Get(int32 X, int32 Y) { return Cells[CELL_INDEX(X, Y)]; }
    FORCEINLINE const FGridFlowTilemapCell& Get(int32 X, int32 Y) const { return Cells[CELL_INDEX(X, Y)]; }

    FORCEINLINE FGridFlowTilemapEdge& GetEdgeH(int32 X, int32 Y) { return EdgesHorizontal[EDGE_INDEX(X, Y)]; }
    FORCEINLINE const FGridFlowTilemapEdge& GetEdgeH(int32 X, int32 Y) const { return EdgesHorizontal[EDGE_INDEX(X, Y)]; }
    
    FORCEINLINE FGridFlowTilemapEdge& GetEdgeV(int32 X, int32 Y) { return EdgesVertical[EDGE_INDEX(X, Y)]; }
    FORCEINLINE const FGridFlowTilemapEdge& GetEdgeV(int32 X, int32 Y) const { return EdgesVertical[EDGE_INDEX(X, Y)]; }
    
    FORCEINLINE void Set(int32 X, int32 Y, const FGridFlowTilemapCell& Value) {
        check(Value.TileCoord == FIntPoint(X, Y));
        Cells[CELL_INDEX(X, Y)] = Value;
    }
    FORCEINLINE void SetEdgeH(int32 X, int32 Y, const FGridFlowTilemapEdge& Value) {
        check(Value.EdgeCoord == FGridFlowTilemapCoord(FIntPoint(X, Y), true));
        EdgesHorizontal[EDGE_INDEX(X, Y)] = Value;
    }
    FORCEINLINE void SetEdgeV(int32 X, int32 Y, const FGridFlowTilemapEdge& Value) {
        check(Value.EdgeCoord == FGridFlowTilemapCoord(FIntPoint(X, Y), false));
        EdgesVertical[EDGE_INDEX(X, Y)] = Value;
    }
    
    FORCEINLINE FGridFlowTilemapCell* GetSafe(int32 X, int32 Y) { return CELL_INDEX_VALID(X, Y) ? &Cells[CELL_INDEX(X, Y)] : nullptr; }
    FORCEINLINE const FGridFlowTilemapCell* GetSafe(int32 X, int32 Y) const { return CELL_INDEX_VALID(X, Y) ? &Cells[CELL_INDEX(X, Y)] : nullptr; }

    FORCEINLINE FGridFlowTilemapEdge* GetEdgeHSafe(int32 X, int32 Y) { return EdgesHorizontal.Num() > 0 && EDGE_INDEX_VALID(X, Y) ? &EdgesHorizontal[EDGE_INDEX(X, Y)] : nullptr; }
    FORCEINLINE const FGridFlowTilemapEdge* GetEdgeHSafe(int32 X, int32 Y) const { return EdgesHorizontal.Num() > 0 && EDGE_INDEX_VALID(X, Y) ? &EdgesHorizontal[EDGE_INDEX(X, Y)] : nullptr; }
    
    FORCEINLINE FGridFlowTilemapEdge* GetEdgeVSafe(int32 X, int32 Y) { return EdgesVertical.Num() > 0 && EDGE_INDEX_VALID(X, Y) ? &EdgesVertical[EDGE_INDEX(X, Y)] : nullptr; }
    FORCEINLINE const FGridFlowTilemapEdge* GetEdgeVSafe(int32 X, int32 Y) const { return EdgesVertical.Num() > 0 && EDGE_INDEX_VALID(X, Y) ? &EdgesVertical[EDGE_INDEX(X, Y)] : nullptr; }

    FORCEINLINE TArray<FGridFlowTilemapCell>& GetCells() { return Cells; }
    FORCEINLINE const TArray<FGridFlowTilemapCell>& GetCells() const { return Cells; }
    FORCEINLINE TArray<FGridFlowTilemapEdge>& GetEdgesH() { return EdgesHorizontal; }
    FORCEINLINE const TArray<FGridFlowTilemapEdge>& GetEdgesH() const { return EdgesHorizontal; }
    FORCEINLINE TArray<FGridFlowTilemapEdge>& GetEdgesV() { return EdgesVertical; }
    FORCEINLINE const TArray<FGridFlowTilemapEdge>& GetEdgesV() const { return EdgesVertical; }
    FORCEINLINE int32 GetWidth() const { return Width; }
    FORCEINLINE int32 GetHeight() const { return Height; }

    void SetWallMetadata(const FGridFlowTilemapCoord& Coord, const FGridFlowTilemapCellWallInfo& InWallMeta);
    void SetDoorMetadata(const FGridFlowTilemapCoord& Coord, const FGridFlowTilemapCellDoorInfo& InDoorMeta);
    bool GetWallMeta(const FGridFlowTilemapCoord& Coord, FGridFlowTilemapCellWallInfo& OutData) const;
    bool GetDoorMeta(const FGridFlowTilemapCoord& Coord, FGridFlowTilemapCellDoorInfo& OutData) const;

    const TMap<FGridFlowTilemapCoord, FGridFlowTilemapCellWallInfo>& GetWallMetaMap() const { return WallMetadataMap; }
    const TMap<FGridFlowTilemapCoord, FGridFlowTilemapCellDoorInfo>& GetDoorMetaMap() const { return DoorMetadataMap; }

    static const FName StateTypeID;
    
private:
    FORCEINLINE int32 CELL_INDEX(int32 X, int32 Y) const { return INDEX_IMPL(X, Y, Width); }
    FORCEINLINE bool CELL_INDEX_VALID(int32 X, int32 Y) const { return INDEX_VALID_IMPL(X, Y, Width, Height); }
    FORCEINLINE int32 EDGE_INDEX(int32 X, int32 Y) const { return INDEX_IMPL(X, Y, Width + 1); }
    FORCEINLINE bool EDGE_INDEX_VALID(int32 X, int32 Y) const { return INDEX_VALID_IMPL(X, Y, Width + 1, Height + 1); }

    FORCEINLINE int32 INDEX_IMPL(int32 X, int32 Y, int32 InWidth) const { return Y * InWidth + X; }
    FORCEINLINE bool INDEX_VALID_IMPL(int32 X, int32 Y, int32 InWidth, int32 InHeight) const { return X >= 0 && Y >= 0 && X < InWidth && Y < InHeight; }
private:
    UPROPERTY()
    TArray<FGridFlowTilemapCell> Cells;

    UPROPERTY()
    TArray<FGridFlowTilemapEdge> EdgesHorizontal;

    UPROPERTY()
    TArray<FGridFlowTilemapEdge> EdgesVertical;

    UPROPERTY()
    int32 Width = 0;

    UPROPERTY()
    int32 Height = 0;

    UPROPERTY()
    TMap<FGridFlowTilemapCoord, FGridFlowTilemapCellWallInfo> WallMetadataMap;

    UPROPERTY()
    TMap<FGridFlowTilemapCoord, FGridFlowTilemapCellDoorInfo> DoorMetadataMap;

};


struct FGridFlowTilemapDistanceFieldCell {
    int32 DistanceFromEdge = MAX_int32;
    int32 DistanceFromDoor = MAX_int32;
};

class DUNGEONARCHITECTRUNTIME_API FGridFlowTilemapDistanceField
    : public TDAArray2D<FGridFlowTilemapDistanceFieldCell> {
public:
    FGridFlowTilemapDistanceField(UGridFlowTilemap* Tilemap);

private:
    void FindDistanceFromEdge(UGridFlowTilemap* Tilemap);
    void FindDistanceFromDoor(UGridFlowTilemap* Tilemap);
};

UENUM(BlueprintType)
enum class EGridFlowAbstractNodeRoomType : uint8 {
    Unknown,
    Room,
    Corridor,
    Cave
};

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FGridFlowAbstractNodeTilemapMetadata {
    GENERATED_BODY()

    /** The coordinate in the tilemap where the chunk representing this layout node starts */
    UPROPERTY()
    FIntPoint TileCoordStart = FIntPoint::ZeroValue;

    /** The coordinate in the tilemap where the chunk representing this layout node end */
    UPROPERTY()
    FIntPoint TileCoordEnd = FIntPoint::ZeroValue;

    UPROPERTY()
    TArray<FIntPoint> Tiles;
};

/**
    Tilemap domain specific data that is attached to the abstract graph nodes
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UFANodeTilemapDomainData : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY()
    FGridFlowAbstractNodeTilemapMetadata TilemapMetadata;
    
    UPROPERTY()
    bool bDisablePerturb = false;

    UPROPERTY()
    EGridFlowAbstractNodeRoomType RoomType = EGridFlowAbstractNodeRoomType::Unknown;
};

