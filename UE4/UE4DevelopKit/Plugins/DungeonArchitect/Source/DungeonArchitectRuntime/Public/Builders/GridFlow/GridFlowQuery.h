//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "GridFlowQuery.generated.h"

class UDungeonBuilder;
class UGridFlowConfig;
class UGridFlowModel;

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FGridFlowChunkQueryResult {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FIntPoint LayoutNodeCoord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FVector TilemapCoordStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FVector TilemapCoordEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<FVector> TileCoords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    EGridFlowAbstractNodeRoomType RoomType;
};

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridFlowQuery : public UDungeonQuery {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    FVector ConvertTileToWorldCoord(const FVector& TileCoord);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    FVector ConvertWorldToTileCoord(const FVector& WorldCoords);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    bool GetCellAtTileCoord(const FVector& TileCoord, FGridFlowTilemapCell& OutCell);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    bool GetCellAtWorldCoord(const FVector& WorldCoord, FGridFlowTilemapCell& OutCell);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    bool GetCellRoomType(const FVector& WorldCoord, EGridFlowAbstractNodeRoomType& OutRoomType);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    void GetFreeTileLocation(TArray<EGridFlowAbstractNodeRoomType> AllowedRoomTypes, const FRandomStream& Random,
                             bool& OutSuccess, FVector& OutWorldCoord);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    bool GetChunkAtWorldCoord(const FVector& InWorldCoord, FGridFlowChunkQueryResult& Result);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    static void GetChunkAtLayoutNode(const UFlowAbstractNode* InLayoutNode, FGridFlowChunkQueryResult& Result);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    bool GetChunkAtLayoutNodeCoord(const FVector& InLayoutNodeCoord, FGridFlowChunkQueryResult& Result) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    void GetAllChunksOfType(EGridFlowAbstractNodeRoomType RoomType, TArray<FGridFlowChunkQueryResult>& OutChunks) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static void IsNearMarker(const FTransform& CurrentMarkerTransform, const FString& NearbyMarkerName, float NearbyDistance,
                             UDungeonBuilder* Builder, bool& bIsNear, int32& NumFound);

protected:
    virtual void InitializeImpl(UDungeonConfig* InConfig, UDungeonModel* InModel) override;

private:
    UPROPERTY()
    UGridFlowConfig* Config;

    UPROPERTY()
    UGridFlowModel* Model;
};

