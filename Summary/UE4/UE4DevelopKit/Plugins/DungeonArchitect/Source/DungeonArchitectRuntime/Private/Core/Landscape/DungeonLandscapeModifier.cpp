//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Landscape/DungeonLandscapeModifier.h"

#include "Builders/Grid/GridDungeonBuilder.h"
#include "Builders/Grid/GridDungeonConfig.h"
#include "Builders/Grid/GridDungeonModel.h"
#include "Builders/SimpleCity/SimpleCityConfig.h"
#include "Builders/SimpleCity/SimpleCityModel.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/MathUtils.h"

#include "AI/NavigationSystemBase.h"
#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeInfo.h"
#include "NavigationSystem.h"

DEFINE_LOG_CATEGORY(LogLandscapeModifier)

UDungeonLandscapeModifier::UDungeonLandscapeModifier(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer), PaintBlurWeightCurve(nullptr) {

}

void UDungeonLandscapeModifier::OnDungeonLayoutBuilt_Implementation(ADungeon* Dungeon) {
#if WITH_EDITOR
    BuildLandscape(Dungeon);
#endif // WITH_EDITOR
}

class FDungeonLandscapeDataRasterizer {
public:
    FDungeonLandscapeDataRasterizer(int32 InSizeX, int32 InSizeY, const FTransform& InLocalToWorld)
        : SizeX(InSizeX), SizeY(InSizeY), LocalToWorld(InLocalToWorld) {
        const int32 ArrayCount = SizeX * SizeY;
        HeightData.AddUninitialized(ArrayCount);
        BlurWeights.AddZeroed(ArrayCount);
    }

    void DrawRect(const FVector& Location, const FVector& Size, float Elevation, float BlurWeight = 1) {
        FVector Start = Location;
        FVector End = Location + Size;

        FVector LocalStart = LocalToWorld.InverseTransformPosition(Start);
        FVector LocalEnd = LocalToWorld.InverseTransformPosition(End);

        int32 iu0 = FMath::RoundToInt(LocalStart.X);
        int32 iv0 = FMath::RoundToInt(LocalStart.Y);
        int32 iu1 = FMath::RoundToInt(LocalEnd.X);
        int32 iv1 = FMath::RoundToInt(LocalEnd.Y);

        float* HeightDataPtr = HeightData.GetData();
        float* BlurWeightsPtr = BlurWeights.GetData();
        for (int iu = iu0; iu <= iu1; iu++) {
            for (int iv = iv0; iv <= iv1; iv++) {
                int32 Index = GetIndex(iu, iv);
                HeightDataPtr[Index] = Elevation;
                BlurWeightsPtr[Index] = BlurWeight;
            }
        }
    }

    void Fill(float Elevation, float BlurWeight = 0) {
        float* HeightDataPtr = HeightData.GetData();
        float* BlurWeightsPtr = BlurWeights.GetData();
        const int32 ArrayCount = SizeX * SizeY;
        for (int i = 0; i < ArrayCount; i++) {
            HeightDataPtr[i] = Elevation;
            BlurWeightsPtr[i] = BlurWeight;
        }
    }

    void Blur(float BlurRadius, int32 BlurIterations) {
        TArray<float> BlurData;
        const int32 ArrayCount = SizeX * SizeY;
        BlurData.AddUninitialized(ArrayCount);

        float* BlurDataPtr = BlurData.GetData();
        float* HeightDataPtr = HeightData.GetData();
        float* BlurWeightsPtr = BlurWeights.GetData();

        TArray<int32> BlurKernels = BlurUtils::boxesForGauss(BlurRadius, BlurIterations);
        if (BlurKernels.Num() > 0) BlurUtils::boxBlur_4(HeightDataPtr, BlurDataPtr, BlurWeightsPtr, SizeX, SizeY,
                                                        (BlurKernels[0] - 1) / 2);
        if (BlurKernels.Num() > 1) BlurUtils::boxBlur_4(BlurDataPtr, HeightDataPtr, BlurWeightsPtr, SizeX, SizeY,
                                                        (BlurKernels[1] - 1) / 2);
        if (BlurKernels.Num() > 2) BlurUtils::boxBlur_4(HeightDataPtr, BlurDataPtr, BlurWeightsPtr, SizeX, SizeY,
                                                        (BlurKernels[2] - 1) / 2);

        for (int i = 0; i < ArrayCount; i++) {
            HeightDataPtr[i] = BlurDataPtr[i];
        }
    }

    void WriteLandscapeHeight(uint16* LandscapeHeightPtr) const {
        int32 ArrayCount = SizeX * SizeY;
        const float* HeightDataPtr = HeightData.GetData();
        for (int i = 0; i < ArrayCount; i++) {
            LandscapeHeightPtr[i] = GetLocalHeight(HeightDataPtr[i]);
        }
    }

    FORCEINLINE uint16 GetLocalHeight(float WorldHeight) const {
        float ElevationScaled = (WorldHeight - LocalToWorld.GetTranslation().Z) / LocalToWorld.GetScale3D().Z;
        return LandscapeDataAccess::GetTexHeight(ElevationScaled);
    }

    FORCEINLINE int32 GetIndex(int32 x, int32 y) {
        return y * SizeX + x;
    }

    float* GetHeightData() { return HeightData.GetData(); }

private:
    int32 SizeX;
    int32 SizeY;
    TArray<float> HeightData;
    TArray<float> BlurWeights;
    FTransform LocalToWorld;
};


#if WITH_EDITOR
void UDungeonLandscapeModifier::BuildLandscape(ADungeon* Dungeon) {
    if (!Landscape) {
        UE_LOG(LogLandscapeModifier, Warning, TEXT("No landscape reference specified in the dungeon actor"));
        return;
    }

    if (!Dungeon) {
        UE_LOG(LogLandscapeModifier, Warning, TEXT("Invalid dungeon actor reference"));
        return;
    }

    ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
    if (!LandscapeInfo) {
        UE_LOG(LogLandscapeModifier, Warning,
               TEXT("Landscape modification works only within the editor (UE4 engine limitation)"));
        return;
    }

    // Build the height data
    RasterizeLayout(Dungeon, LandscapeInfo);

    RasterizeWeights(Dungeon, LandscapeInfo);


    BuildCollision(LandscapeInfo, Dungeon->GetWorld());
}

void UDungeonLandscapeModifier::RasterizeLayout(ADungeon* Dungeon, ULandscapeInfo* LandscapeInfo) {
    if (!Dungeon) {
        return;
    }
    
    int32 MinX = MAX_int32;
    int32 MinY = MAX_int32;
    int32 MaxX = -MAX_int32;
    int32 MaxY = -MAX_int32;
    if (!LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY)) {
        return;
    }

    if (!LandscapeInfo->GetLandscapeProxy()) {
        return;
    }

    int32 TargetSizeX = MaxX - MinX + 1;
    int32 TargetSizeY = MaxY - MinY + 1;

    FTransform LocalToWorld = LandscapeInfo->GetLandscapeProxy()->ActorToWorld();

    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
    FDungeonLandscapeDataRasterizer Rasterizer(TargetSizeX, TargetSizeY, LocalToWorld);
    Rasterizer.Fill(BaseHeight);

    bool bProcessed = false;

    // Grid builder rasterizer
    {
        UGridDungeonModel* GridModel = Cast<UGridDungeonModel>(Dungeon->GetModel());
        UGridDungeonConfig* GridConfig = Cast<UGridDungeonConfig>(Dungeon->GetConfig());

        if (GridModel && GridConfig) {
            RasterizeLayoutGridBuilder(GridModel, GridConfig, Rasterizer, HeightBlurWeight, false, true, true);
            bProcessed = true;
        }
    }

    // Grid builder rasterizer
    if (!bProcessed) {
        USimpleCityModel* CityModel = Cast<USimpleCityModel>(Dungeon->GetModel());
        USimpleCityConfig* CityConfig = Cast<USimpleCityConfig>(Dungeon->GetConfig());
        if (CityModel && CityConfig) {
            FVector DungeonLocation = Dungeon ? Dungeon->GetActorLocation() : FVector::ZeroVector;
            RasterizeLayoutCityBuilder(CityModel, CityConfig, Rasterizer, DungeonLocation, HeightBlurWeight);
            bProcessed = true;
        }
    }


    Rasterizer.Blur(HeightBlurRadius, HeightBlurIterations);

    // Prepare the landscape height data

    TArray<uint16> HeightData;
    int32 ArrayCount = TargetSizeX * TargetSizeY;
    HeightData.AddUninitialized(ArrayCount);
    uint16* HeightDataPtr = HeightData.GetData();
    Rasterizer.WriteLandscapeHeight(HeightDataPtr);

    // Set the new height data and flush 
    LandscapeEdit.SetHeightData(MinX, MinY, MaxX, MaxY, HeightDataPtr, 0, true, nullptr, nullptr, nullptr, true);
    LandscapeEdit.Flush();
}

void UDungeonLandscapeModifier::RasterizeLayoutGridBuilder(UGridDungeonModel* GridModel, UGridDungeonConfig* GridConfig,
                                                           FDungeonLandscapeDataRasterizer& Rasterizer,
                                                           float RasterizeBlurWeight, bool bMaskLayout,
                                                           bool bRasterizeRooms, bool bRasterizeCorridors) {
    FVector GridSize = GridConfig->GridCellSize;
    for (const FCell& Cell : GridModel->Cells) {
        if (Cell.CellType == FCellType::Room && !bRasterizeRooms) continue;
        if ((Cell.CellType == FCellType::Corridor || Cell.CellType == FCellType::CorridorPadding) && !
            bRasterizeCorridors) continue;

        FVector Location = GridSize * FVector(Cell.Bounds.Location);
        FVector Size = GridSize * FVector(Cell.Bounds.Size);
        float Elevation = Location.Z;
        if (bMaskLayout) {
            Elevation = 1.0f;
        }

        Rasterizer.DrawRect(Location, Size, Elevation, RasterizeBlurWeight);
    }
}

void UDungeonLandscapeModifier::RasterizeLayoutCityBuilder(USimpleCityModel* CityModel, USimpleCityConfig* CityConfig,
                                                           FDungeonLandscapeDataRasterizer& Rasterizer,
                                                           const FVector& DungeonLocation, float RasterizeBlurWeight) {
    const int32 Width = CityModel->CityWidth;
    const int32 Length = CityModel->CityLength;
    const FVector CellSize = FVector(CityConfig->CellSize.X, CityConfig->CellSize.Y, 0);
    FVector BasePosition = DungeonLocation;

    for (int x = 0; x < Width; x++) {
        for (int y = 0; y < Length; y++) {
            int32 Index = y * Width + x;
            FSimpleCityCell& Cell = CityModel->Cells[Index];
            bool bRasterizeCell = false;
            if (Cell.CellType == ESimpleCityCellType::Road) {
                bRasterizeCell = true;
                RasterizeBlurWeight = 1.0f;
            }
            else if (Cell.CellType == ESimpleCityCellType::House || Cell.CellType == ESimpleCityCellType::UserDefined) {
                bRasterizeCell = true;
                RasterizeBlurWeight = 0.5f;
            }

            if (bRasterizeCell) {
                FVector CellPositionLogical = UDungeonModelHelper::MakeVector(Cell.Position);
                FVector BlockSizeLogical = UDungeonModelHelper::MakeVector(Cell.BlockSize);
                FVector WorldSize = BlockSizeLogical * CellSize;
                FVector WorldLocation = CellPositionLogical * CellSize + BasePosition;
                const float Elevation = 0.0f;

                Rasterizer.DrawRect(WorldLocation, WorldSize, WorldLocation.Z, RasterizeBlurWeight);
            }
        }
    }
}

void UDungeonLandscapeModifier::RasterizeWeights(ADungeon* Dungeon, ULandscapeInfo* LandscapeInfo) {
    int32 MinX = MAX_int32;
    int32 MinY = MAX_int32;
    int32 MaxX = -MAX_int32;
    int32 MaxY = -MAX_int32;
    if (!LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY)) {
        return;
    }

    if (!LandscapeInfo->GetLandscapeProxy()) {
        return;
    }

    int32 TargetSizeX = MaxX - MinX + 1;
    int32 TargetSizeY = MaxY - MinY + 1;
    FTransform LocalToWorld = LandscapeInfo->GetLandscapeProxy()->ActorToWorld();

    // Fill default material
    FDungeonLandscapeDataRasterizer Rasterizer(TargetSizeX, TargetSizeY, LocalToWorld);
    Rasterizer.Fill(0);

    // Grid builder rasterizer
    {
        UGridDungeonModel* GridModel = Cast<UGridDungeonModel>(Dungeon->GetModel());
        UGridDungeonConfig* GridConfig = Cast<UGridDungeonConfig>(Dungeon->GetConfig());

        if (GridModel && GridConfig) {
            RasterizeLayoutGridBuilder(GridModel, GridConfig, Rasterizer, PaintBlurWeight, true, true, true);
        }
    }

    Rasterizer.Blur(PaintBlurRadius, PaintBlurIterations);

    const int32 ArrayCount = TargetSizeX * TargetSizeY;
    TArray<uint8> WeightFill;
    WeightFill.AddUninitialized(ArrayCount);
    uint8* WeightFillPtr = WeightFill.GetData();

    TArray<uint8> WeightLayout;
    WeightLayout.AddUninitialized(ArrayCount);
    uint8* WeightLayoutPtr = WeightLayout.GetData();

    float* BaseWeightPtr = Rasterizer.GetHeightData();
    for (int i = 0; i < ArrayCount; i++) {
        float Weight = FMath::Clamp(BaseWeightPtr[i], 0.0f, 1.0f);
        if (PaintBlurWeightCurve) {
            Weight = PaintBlurWeightCurve->GetFloatValue(Weight);
        }
        WeightLayoutPtr[i] = FMath::RoundToInt(Weight * 255);
        WeightFillPtr[i] = 255 - WeightLayoutPtr[i];
    }

    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
    if (Layers.Num() > 0) {
        LandscapeEdit.SetAlphaData(Layers[0], MinX, MinY, MaxX, MaxY, WeightFillPtr, 0);
    }
    if (Layers.Num() > 1) {
        LandscapeEdit.SetAlphaData(Layers[1], MinX, MinY, MaxX, MaxY, WeightLayoutPtr, 0);
    }

    LandscapeEdit.Flush();
}

void UDungeonLandscapeModifier::BuildCollision(ULandscapeInfo* LandscapeInfo, UWorld* InWorld) {
    int32 MinX = MAX_int32;
    int32 MinY = MAX_int32;
    int32 MaxX = -MAX_int32;
    int32 MaxY = -MAX_int32;
    if (!LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY)) {
        return;
    }

    int32 TargetSizeX = MaxX - MinX + 1;
    int32 TargetSizeY = MaxY - MinY + 1;

    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
    TSet<ULandscapeComponent*> Components;
    if (LandscapeEdit.GetComponentsInRegion(MinX, MinY, MaxX, MaxY, &Components) && Components.Num() > 0) {
        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(InWorld);

        for (ULandscapeComponent* Component : Components) {
            // Recreate collision for modified components and update the navmesh
            ULandscapeHeightfieldCollisionComponent* CollisionComponent = Component->CollisionComponent.Get();
            if (CollisionComponent) {
                CollisionComponent->RecreateCollision();
                if (NavSys) {
                    NavSys->UpdateComponentInNavOctree(*CollisionComponent);
                }
            }
        }

        // Flush dynamic data like grass
        ALandscapeProxy::InvalidateGeneratedComponentData(Components);
    }
}

#endif	// WITH_EDITOR

