//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/SimpleCity/SimpleCityModel.h"
#include "Core/DungeonBuilder.h"
#include "Core/DungeonModel.h"
#include "Core/Utils/PMRandom.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"
#include "SimpleCityBuilder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SimpleCityBuilderLog, Log, All);

class ADungeon;
class USimpleCityConfig;
class USimpleCityModel;
class USimpleCitySpatialConstraint3x3;

struct FLevelGrowthNode;
/**
*
*/
UCLASS(Experimental)
class DUNGEONARCHITECTRUNTIME_API USimpleCityBuilder : public UDungeonBuilder {
    GENERATED_BODY()

public:
    virtual void BuildDungeonImpl(UWorld* World) override;
    virtual void EmitDungeonMarkers_Implementation() override;
    virtual void DrawDebugData(UWorld* InWorld, bool bPersistant = false, float lifeTime = -1.0f) override;
    virtual bool SupportsBackgroundTask() const override { return false; }
    virtual void MirrorDungeon() override;
    virtual TSubclassOf<UDungeonModel> GetModelClass() override;
    virtual TSubclassOf<UDungeonConfig> GetConfigClass() override;
    virtual TSubclassOf<UDungeonToolData> GetToolDataClass() override;
    virtual TSubclassOf<UDungeonQuery> GetQueryClass() override;
    virtual bool ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform,
                                          FQuat& OutRotationOffset) override;
    virtual void GetDefaultMarkerNames(TArray<FString>& OutMarkerNames) override;

private:
    void GenerateCityLayout();
    void FaceHouseTowardsRoad(FSimpleCityCell& Cell);

    FQuat GetRandomRotation();
    int GetRandomBlockSize();
    bool CanContainBiggerHouse(int x, int y, int w, int h);
    void InsertBiggerHouse(int x, int y, int w, int h, float Angle, const FString& MarkerName);

    void MakeRoad(int32 x, int32 y);
    void RemoveRoadEdges();

    bool IsStraightRoad(int x, int y);
    void RemoveRoadEdge(int x, int y);
    ESimpleCityCellType GetCellType(int x, int y);


protected:

    virtual void MirrorDungeonWithVolume(ADungeonMirrorVolume* MirrorVolume) override;
    virtual bool PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics,
                                       const FPropSocket& socket) override;
    virtual FTransform PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics,
                                             const FPropSocket& socket) override;
    bool ProcessSpatialConstraint3x3(USimpleCitySpatialConstraint3x3* SpatialConstraint, const FTransform& Transform,
                                     FQuat& OutRotationOffset);

private:
    USimpleCityModel* CityModel;
    USimpleCityConfig* CityConfig;
};

