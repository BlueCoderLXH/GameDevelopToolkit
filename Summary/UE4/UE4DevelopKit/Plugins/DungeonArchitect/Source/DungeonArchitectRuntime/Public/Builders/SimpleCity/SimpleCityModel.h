//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonModel.h"
#include "SimpleCityModel.generated.h"

UENUM(BlueprintType)
enum class ESimpleCityCellType : uint8 {
    Road UMETA(DisplayName = "Road"),
    House UMETA(DisplayName = "House"),
    Park UMETA(DisplayName = "Park"),
    UserDefined UMETA(DisplayName = "UserDefined"),
    Empty UMETA(DisplayName = "Empty")
};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FSimpleCityCell {
    GENERATED_USTRUCT_BODY()

    FSimpleCityCell() : Position(FIntVector::ZeroValue), CellType(ESimpleCityCellType::Empty),
                        Rotation(FQuat::Identity), BlockSize(FIntVector(1, 1, 0)) {
    }

    UPROPERTY()
    FIntVector Position;

    UPROPERTY()
    ESimpleCityCellType CellType;

    UPROPERTY()
    FQuat Rotation;

    UPROPERTY()
    FIntVector BlockSize;

    UPROPERTY()
    FString MarkerNameOverride;
};


USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FCityBlockDimension {
    GENERATED_USTRUCT_BODY()

    FCityBlockDimension()
        : MarkerName("HouseNxN")
          , SizeX(1)
          , SizeY(1)
          , Probability(0) {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FString MarkerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 SizeX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 SizeY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float Probability;
};

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USimpleCityModel : public UDungeonModel {
    GENERATED_BODY()
public:

    UPROPERTY(BlueprintReadOnly, Category = Dungeon)
    int32 CityWidth;

    UPROPERTY(BlueprintReadOnly, Category = Dungeon)
    int32 CityLength;

    UPROPERTY(BlueprintReadOnly, Category = Dungeon)
    TArray<FSimpleCityCell> Cells;

public:
    virtual void Reset() override;

    FORCEINLINE int32 CELL_INDEX(int32 x, int32 y) { return y * CityWidth + x; }

};

