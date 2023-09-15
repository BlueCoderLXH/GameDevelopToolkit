//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SimpleCity/SimpleCityBuilder.h"

#include "Builders/SimpleCity/SimpleCityConfig.h"
#include "Builders/SimpleCity/SimpleCityModel.h"
#include "Builders/SimpleCity/SimpleCityQuery.h"
#include "Builders/SimpleCity/SimpleCitySelectorLogic.h"
#include "Builders/SimpleCity/SimpleCityToolData.h"
#include "Builders/SimpleCity/SimpleCityTransformLogic.h"
#include "Builders/SimpleCity/SpatialConstraints/SimpleCitySpatialConstraint3x3.h"
#include "Builders/SimpleCity/Utils/SimpleCityConstants.h"
#include "Builders/SimpleCity/Utils/SimpleCityRoadBeautifier.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/SpatialConstraintUtils.h"
#include "Core/Volumes/DungeonMirrorVolume.h"

#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(SimpleCityBuilderLog);
#define CITY_INDEX(x, y) ((y) * CityModel->CityWidth + (x))


void USimpleCityBuilder::BuildDungeonImpl(UWorld* World) {
    CityModel = Cast<USimpleCityModel>(model);
    CityConfig = Cast<USimpleCityConfig>(config);

    if (!CityModel) {
        UE_LOG(SimpleCityBuilderLog, Error, TEXT("Invalid dungeon model provided to the grid builder"));
        return;
    }

    if (!CityConfig) {
        UE_LOG(SimpleCityBuilderLog, Error, TEXT("Invalid dungeon gridConfig provided to the grid builder"));
        return;
    }

    GenerateCityLayout();

    PropSockets.Reset();
}


void USimpleCityBuilder::GenerateCityLayout() {
    CityModel->CityWidth = random.RandRange(CityConfig->MinCitySize, CityConfig->MaxCitySize);
    CityModel->CityLength = random.RandRange(CityConfig->MinCitySize, CityConfig->MaxCitySize);

    const int32 Width = CityModel->CityWidth;
    const int32 Length = CityModel->CityLength;

    int32 NumCells = Width * Length;
    CityModel->Cells.Reset();
    CityModel->Cells.AddDefaulted(NumCells);

    for (int x = 0; x < Width; x++) {
        for (int y = 0; y < Length; y++) {
            FSimpleCityCell& Cell = CityModel->Cells[CITY_INDEX(x, y)];
            Cell.Position = FIntVector(x, y, 0);
            Cell.CellType = ESimpleCityCellType::House; // Later we will overwrite some with roads
            Cell.Rotation = GetRandomRotation();
        }
    }

    // Build a road network by removing some houses 
    // First build roads along the edge of the map
    for (int x = 0; x < Width; x++) {
        MakeRoad(x, 0);
        MakeRoad(x, Length - 1);
    }
    for (int z = 0; z < Length; z++) {
        MakeRoad(0, z);
        MakeRoad(Width - 1, z);
    }

    // Create roads in-between
    for (int x = GetRandomBlockSize() + 1; x < Width; x += GetRandomBlockSize() + 1) {
        if (Width - x <= 2) continue;
        for (int z = 0; z < Length; z++) {
            MakeRoad(x, z);
        }
    }

    for (int z = GetRandomBlockSize() + 1; z < Length; z += GetRandomBlockSize() + 1) {
        if (Length - z <= 2) continue;
        for (int x = 0; x < Width; x++) {
            MakeRoad(x, z);
        }
    }

    // Remove roads to create a non-uniform city
    RemoveRoadEdges();

    // Face the houses toward the road, or convert them to parks
    for (int x = 0; x < Width; x++) {
        for (int y = 0; y < Length; y++) {
            FSimpleCityCell& Cell = CityModel->Cells[CITY_INDEX(x, y)];
            if (Cell.CellType == ESimpleCityCellType::House) {
                FaceHouseTowardsRoad(Cell);
            }
        }
    }

    // Insert larger houses (with user defined dimensions)
    for (int x = 0; x < Width; x++) {
        for (int z = 0; z < Length; z++) {
            // Iterate through each custom block dimension
            for (const FCityBlockDimension& BlockDimension : CityConfig->CityBlockDimensions) {
                bool bProcess = random.FRand() < BlockDimension.Probability;
                if (bProcess) {
                    int32 BlockWidth = BlockDimension.SizeX;
                    int32 BlockHeight = BlockDimension.SizeY;

                    auto InsertHouse = [&]() {
                        if (CanContainBiggerHouse(x, z, BlockWidth, BlockHeight)) {
                            if (random.FRand() < CityConfig->BiggerHouseProbability) {
                                InsertBiggerHouse(x, z, BlockWidth, BlockHeight, 0, BlockDimension.MarkerName);
                            }
                        }
                    };


                    auto InsertHouse90 = [&]() {
                        // Try the 90 degrees rotated version
                        if (CanContainBiggerHouse(x, z, BlockHeight, BlockWidth)) {
                            if (random.FRand() < CityConfig->BiggerHouseProbability) {
                                InsertBiggerHouse(x, z, BlockHeight, BlockWidth, 90, BlockDimension.MarkerName);
                            }
                        }
                    };

                    if (random.FRand() < 0.5f) {
                        InsertHouse();
                        InsertHouse90();
                    }
                    else {
                        InsertHouse90();
                        InsertHouse();
                    }
                }
            }
        }
    }
}

void USimpleCityBuilder::FaceHouseTowardsRoad(FSimpleCityCell& Cell) {
    int x = Cell.Position.X;
    int y = Cell.Position.Y;

    bool roadLeft = GetCellType(x - 1, y) == ESimpleCityCellType::Road;
    bool roadRight = GetCellType(x + 1, y) == ESimpleCityCellType::Road;
    bool roadTop = GetCellType(x, y - 1) == ESimpleCityCellType::Road;
    bool roadBottom = GetCellType(x, y + 1) == ESimpleCityCellType::Road;

    if (!roadLeft && !roadRight && !roadTop && !roadBottom) {
        Cell.CellType = ESimpleCityCellType::Park;
        // interior
        return;
    }

    float angle = 0;
    if (roadLeft) angle = 180;
    else if (roadRight) angle = 0;
    else if (roadTop) angle = 270;
    else if (roadBottom) angle = 90;

    Cell.Rotation = FQuat::MakeFromEuler(FVector(0, 0, angle));
}

void USimpleCityBuilder::RemoveRoadEdges() {
    const int32 Width = CityModel->CityWidth;
    const int32 Length = CityModel->CityLength;
    for (int x = 0; x < Width; x++) {
        for (int y = 0; y < Length; y++) {
            if (IsStraightRoad(x, y)) {
                bool bRemove = random.FRand() < CityConfig->RoadEdgeRemovalProbability;
                if (bRemove) {
                    RemoveRoadEdge(x, y);
                }
            }
        }
    }

    // Remove the isolated road cells
    for (int x = 0; x < Width; x++) {
        for (int y = 0; y < Length; y++) {
            if (GetCellType(x, y) == ESimpleCityCellType::Road) {
                int32 Adjacent = 0;
                if (GetCellType(x, y - 1) == ESimpleCityCellType::Road) Adjacent++;
                if (GetCellType(x, y + 1) == ESimpleCityCellType::Road) Adjacent++;
                if (GetCellType(x - 1, y) == ESimpleCityCellType::Road) Adjacent++;
                if (GetCellType(x + 1, y) == ESimpleCityCellType::Road) Adjacent++;
                if (Adjacent == 0) {
                    // No adjacent roads connecting to this road cell. remove it
                    FSimpleCityCell& Cell = CityModel->Cells[CITY_INDEX(x, y)];
                    Cell.CellType = ESimpleCityCellType::House;
                }
            }
        }
    }

}

bool USimpleCityBuilder::IsStraightRoad(int x, int y) {
    if (GetCellType(x, y) != ESimpleCityCellType::Road) {
        return false;
    }

    bool bTop = GetCellType(x, y - 1) == ESimpleCityCellType::Road;
    bool bBottom = GetCellType(x, y + 1) == ESimpleCityCellType::Road;
    bool bLeft = GetCellType(x - 1, y) == ESimpleCityCellType::Road;
    bool bRight = GetCellType(x + 1, y) == ESimpleCityCellType::Road;

    bool bHorizontal = bLeft && bRight;
    bool bVertical = bTop && bBottom;

    int32 Adjacent = 0;
    if (bTop) Adjacent++;
    if (bBottom) Adjacent++;
    if (bLeft) Adjacent++;
    if (bRight) Adjacent++;

    if (Adjacent != 2) return false;

    return bHorizontal || bVertical;
}

void USimpleCityBuilder::RemoveRoadEdge(int x, int y) {
    if (!IsStraightRoad(x, y)) {
        // Nothing to remove
        return;
    }

    TSet<FIntVector> RoadsToRemove;
    RoadsToRemove.Add(FIntVector(x, y, 0));
    int index = x - 1;
    while (IsStraightRoad(index, y)) {
        RoadsToRemove.Add(FIntVector(index, y, 0));
        index--;
    }
    index = x + 1;
    while (IsStraightRoad(index, y)) {
        RoadsToRemove.Add(FIntVector(index, y, 0));
        index++;
    }

    index = y - 1;
    while (IsStraightRoad(x, index)) {
        RoadsToRemove.Add(FIntVector(x, index, 0));
        index--;
    }
    index = y + 1;
    while (IsStraightRoad(x, index)) {
        RoadsToRemove.Add(FIntVector(x, index, 0));
        index++;
    }

    for (const FIntVector& Position : RoadsToRemove) {
        FSimpleCityCell& Cell = CityModel->Cells[CITY_INDEX(Position.X, Position.Y)];
        Cell.CellType = ESimpleCityCellType::House;
    }

}

ESimpleCityCellType USimpleCityBuilder::GetCellType(int x, int y) {
    if (x < 0 || y < 0 || x >= CityModel->CityWidth || y >= CityModel->CityLength) {
        return ESimpleCityCellType::Empty;
    }
    return CityModel->Cells[CITY_INDEX(x, y)].CellType;
}

FQuat USimpleCityBuilder::GetRandomRotation() {
    float Angle = random.RandRange(0, 3) * 90;
    return FQuat::MakeFromEuler(FVector(0, 0, Angle));
}

int USimpleCityBuilder::GetRandomBlockSize() {
    return random.RandRange(CityConfig->MinBlockSize, CityConfig->MaxBlockSize);
}

bool USimpleCityBuilder::CanContainBiggerHouse(int x, int y, int w, int h) {
    for (int dx = 0; dx < w; dx++) {
        for (int dy = 0; dy < h; dy++) {
            int32 xx = x + dx;
            int32 yy = y + dy;
            ESimpleCityCellType CellType = ESimpleCityCellType::Empty;
            if (xx < CityModel->CityWidth && yy < CityModel->CityLength) {
                CellType = CityModel->Cells[CITY_INDEX(x + dx, y + dy)].CellType;
            }

            if (CellType != ESimpleCityCellType::House && CellType != ESimpleCityCellType::Park) {
                return false;
            }
        }
    }
    return true;
}

void USimpleCityBuilder::InsertBiggerHouse(int x, int y, int w, int h, float Angle, const FString& MarkerName) {
    for (int dx = 0; dx < w; dx++) {
        for (int dy = 0; dy < h; dy++) {
            FSimpleCityCell& Cell = CityModel->Cells[CITY_INDEX(x + dx, y + dy)];
            if (dx == 0 && dy == 0) {
                Cell.CellType = ESimpleCityCellType::UserDefined;
                Cell.Rotation = FQuat::MakeFromEuler(FVector(0, 0, Angle));
                Cell.BlockSize = FIntVector(w, h, 0);
                Cell.MarkerNameOverride = MarkerName;
            }
            else {
                // Make these cells empty, as they will be occupied by the bigger house and we don't want any markers here
                Cell.CellType = ESimpleCityCellType::Empty;
            }
        }
    }
}

void USimpleCityBuilder::MakeRoad(int32 x, int32 y) {
    FSimpleCityCell& Cell = CityModel->Cells[CITY_INDEX(x, y)];
    Cell.CellType = ESimpleCityCellType::Road;
    Cell.Rotation = FQuat::Identity;
}

void USimpleCityBuilder::EmitDungeonMarkers_Implementation() {
    Super::EmitDungeonMarkers_Implementation();

    CityModel = Cast<USimpleCityModel>(model);
    CityConfig = Cast<USimpleCityConfig>(config);

    ClearSockets();

    FVector BasePosition = Dungeon ? Dungeon->GetActorLocation() : FVector::ZeroVector;
    auto& cells = CityModel->Cells;
    const int32 width = CityModel->CityWidth;
    const int32 length = CityModel->CityLength;
    const FVector CellSize = FVector(CityConfig->CellSize.X, CityConfig->CellSize.Y, 0);


    for (int x = 0; x < width; x++) {
        for (int y = 0; y < length; y++) {
            FSimpleCityCell& Cell = CityModel->Cells[CITY_INDEX(x, y)];
            FVector CellPositionLogical = UDungeonModelHelper::MakeVector(Cell.Position);
            FVector BlockSizeLogical = UDungeonModelHelper::MakeVector(Cell.BlockSize);
            FVector WorldPosition = CellPositionLogical * CellSize + BasePosition;
            FString MarkerName = "Unknown";
            FQuat Rotation = FQuat::Identity;
            if (Cell.CellType == ESimpleCityCellType::House) {
                MarkerName = SimpleCityDungeonConstants::MarkerHouse;
                Rotation = Cell.Rotation;
            }
            if (Cell.CellType == ESimpleCityCellType::Park) {
                MarkerName = SimpleCityDungeonConstants::MarkerPark;
                Rotation = Cell.Rotation;
            }
            else if (Cell.CellType == ESimpleCityCellType::UserDefined) {
                MarkerName = Cell.MarkerNameOverride;
                WorldPosition = (CellPositionLogical + BlockSizeLogical / 2.0f - FVector(0.5f, 0.5f, 0)) * CellSize +
                    BasePosition;
                Rotation = Cell.Rotation;
            }
            else if (Cell.CellType == ESimpleCityCellType::Road) {
                float Angle = 0;
                MarkerName = FSimpleCityRoadBeautifier::GetRoadMarkerName(x, y, CityModel, Angle);
                Rotation = FQuat::MakeFromEuler(FVector(0, 0, Angle));
            }
            FTransform MarkerTransform(Rotation, WorldPosition);
            EmitMarker(MarkerName, MarkerTransform);
        }
    }
}

void USimpleCityBuilder::DrawDebugData(UWorld* InWorld, bool bPersistant /*= false*/, float lifeTime /*= 0*/) {
    if (!CityModel || !CityConfig) return;

    const float DebugCellHeight = 10;
    FVector BasePosition = Dungeon ? Dungeon->GetActorLocation() : FVector::ZeroVector;
    FVector CellSize = FVector(CityConfig->CellSize.X, CityConfig->CellSize.Y, DebugCellHeight);
    FVector HalfCellSize = CellSize / 2.0f;

    for (const FSimpleCityCell& cell : CityModel->Cells) {
        FVector Location = UDungeonModelHelper::MakeVector(cell.Position) * CellSize + BasePosition;

        FColor Color = FColor::White;
        switch (cell.CellType) {

        case ESimpleCityCellType::Road:
            Color = FColor::Black;
            break;

        case ESimpleCityCellType::House:
            Color = FColor::Red;
            break;

        case ESimpleCityCellType::UserDefined:
            Color = FColor::Purple;
            break;

        default:
            Color = FColor::White;
            break;
        }

        FColor SolidColor = Color;
        SolidColor.A = 32;

        DrawDebugSolidBox(InWorld, Location, HalfCellSize, SolidColor, bPersistant, lifeTime);
        DrawDebugBox(InWorld, Location, HalfCellSize, Color, bPersistant, lifeTime);

    }
}

void USimpleCityBuilder::MirrorDungeon() {

}

TSubclassOf<UDungeonModel> USimpleCityBuilder::GetModelClass() {
    return USimpleCityModel::StaticClass();
}

TSubclassOf<UDungeonConfig> USimpleCityBuilder::GetConfigClass() {
    return USimpleCityConfig::StaticClass();
}

TSubclassOf<UDungeonToolData> USimpleCityBuilder::GetToolDataClass() {
    return USimpleCityToolData::StaticClass();
}

TSubclassOf<UDungeonQuery> USimpleCityBuilder::GetQueryClass() {
    return USimpleCityQuery::StaticClass();
}

bool USimpleCityBuilder::ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint,
                                                  const FTransform& Transform, FQuat& OutRotationOffset) {
    if (USimpleCitySpatialConstraint3x3* Constraint = Cast<USimpleCitySpatialConstraint3x3>(SpatialConstraint)) {
        return ProcessSpatialConstraint3x3(Constraint, Transform, OutRotationOffset);
    }

    return false;
}

bool USimpleCityBuilder::ProcessSpatialConstraint3x3(USimpleCitySpatialConstraint3x3* SpatialConstraint,
                                                     const FTransform& Transform, FQuat& OutRotationOffset) {
    FVector2D GridSize2D = CityConfig->CellSize;
    FVector GridSize(GridSize2D.X, GridSize2D.Y, 1);

    if (!SpatialConstraint) return false;
    FVector WorldLoc = Transform.GetLocation();
    FVector GridLocF = WorldLoc / GridSize;
    GridLocF.Z = 0;
    FIntVector GridLoc(
        FMath::FloorToInt(GridLocF.X),
        FMath::FloorToInt(GridLocF.Y),
        FMath::FloorToInt(GridLocF.Z));

    TArray<FSimpleCitySpatialConstraintCellData> Neighbors = SpatialConstraint->Configuration.Cells;
    int RotationsRequired = SpatialConstraint->bRotateToFitConstraint ? 3 : 0;

    for (int RotationStep = 0; RotationStep <= RotationsRequired; RotationStep++) {
        bool ConfigMatches = true;
        for (int i = 0; i < Neighbors.Num(); i++) {
            auto code = Neighbors[i];
            if (code.OccupationConstraint == ESimpleCitySpatialCellOccupation::Ignore) {
                // Don't care about this cell
                continue;
            }
            int32 dx = i % 3;
            int32 dy = i / 3;
            dx--;
            dy--; // bring to -1..1 range (from previous 0..2)
            //dy *= -1;
            int32 x = GridLoc.X + dx;
            int32 y = GridLoc.Y + dy;

            ESimpleCityCellType CellInfo = GetCellType(x, y);
            if (code.OccupationConstraint == ESimpleCitySpatialCellOccupation::Road && CellInfo !=
                ESimpleCityCellType::Road) {
                ConfigMatches = false;
                break;
            }
            if (code.OccupationConstraint == ESimpleCitySpatialCellOccupation::House && (CellInfo !=
                ESimpleCityCellType::House && CellInfo != ESimpleCityCellType::UserDefined)) {
                ConfigMatches = false;
                break;
            }
            if (code.OccupationConstraint == ESimpleCitySpatialCellOccupation::Park && CellInfo !=
                ESimpleCityCellType::Park) {
                ConfigMatches = false;
                break;
            }
            if (code.OccupationConstraint == ESimpleCitySpatialCellOccupation::Outskirts && CellInfo !=
                ESimpleCityCellType::Empty) {
                ConfigMatches = false;
                break;
            }

        }

        if (ConfigMatches) {
            float RotationAngle = -90 * RotationStep;
            OutRotationOffset = FQuat::MakeFromEuler(FVector(0, 0, RotationAngle));
            return true;
        }

        if (RotationStep < RotationsRequired) {
            Neighbors = FSpatialConstraintUtils::RotateNeighborConfig3x3<FSimpleCitySpatialConstraintCellData>(
                Neighbors);
        }
    }

    // No configurations matched
    OutRotationOffset = FQuat::Identity;
    return false;
}

void USimpleCityBuilder::GetDefaultMarkerNames(TArray<FString>& OutMarkerNames) {
    OutMarkerNames.Reset();
    OutMarkerNames.Add(SimpleCityDungeonConstants::MarkerHouse);
    OutMarkerNames.Add(SimpleCityDungeonConstants::MarkerPark);
    OutMarkerNames.Add(SimpleCityDungeonConstants::MarkerRoadX);
    OutMarkerNames.Add(SimpleCityDungeonConstants::MarkerRoadT);
    OutMarkerNames.Add(SimpleCityDungeonConstants::MarkerRoadS);
    OutMarkerNames.Add(SimpleCityDungeonConstants::MarkerRoadCorner);
    OutMarkerNames.Add(SimpleCityDungeonConstants::MarkerRoadEnd);
}

void USimpleCityBuilder::MirrorDungeonWithVolume(ADungeonMirrorVolume* MirrorVolume) {
}

bool USimpleCityBuilder::PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics,
                                               const FPropSocket& socket) {
    return false;
}

FTransform USimpleCityBuilder::PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics,
                                                     const FPropSocket& socket) {
    return FTransform::Identity;
}

#undef CITY_INDEX

