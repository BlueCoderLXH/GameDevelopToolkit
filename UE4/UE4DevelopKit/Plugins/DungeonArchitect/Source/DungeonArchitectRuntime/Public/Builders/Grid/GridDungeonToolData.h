//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonToolData.h"
#include "Core/Utils/Rectangle.h"
#include "GridDungeonToolData.generated.h"

UENUM(BlueprintType)
enum class EGridPaintToolCellType : uint8 {
    Corridor = 0 UMETA(DisplayName = "Corridor"),
    Room = 1 UMETA(DisplayName = "Room")
};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FGridToolPaintStrokeData {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FIntVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    EGridPaintToolCellType CellType;
};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FGridToolRectStrokeData {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FRectangle Rectangle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    EGridPaintToolCellType CellType;

};

bool operator==(const FGridToolRectStrokeData& A, const FGridToolRectStrokeData& B);
bool operator==(const FGridToolPaintStrokeData& A, const FGridToolPaintStrokeData& B);

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridDungeonToolData : public UDungeonToolData {
    GENERATED_BODY()

public:

    // The cells painted by the "Paint" tool
    UPROPERTY()
    TArray<FGridToolPaintStrokeData> PaintedCells;

    // The platform rectangles defined in the scene using the "Rectangle" tool
    UPROPERTY()
    TArray<FGridToolRectStrokeData> Rectangles;

    // The platform borders defined in the scene using the "Border" tool
    UPROPERTY()
    TArray<FGridToolRectStrokeData> Borders;

};

