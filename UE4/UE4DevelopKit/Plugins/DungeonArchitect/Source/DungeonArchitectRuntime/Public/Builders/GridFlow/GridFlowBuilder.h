//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonBuilder.h"
#include "Core/Utils/Attributes.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/FlowProcessor.h"

#include "Components/ActorComponent.h"
#include "GridFlowBuilder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GridFlowBuilderLog, Log, All);

class ADungeon;
class ADungeonVolume;
class UGridFlowConfig;
class UGridFlowQuery;
class UGridFlowModel;

class UGridFlowTilemap;
struct FGridFlowTilemapEdge;

UCLASS(EarlyAccessPreview)
class DUNGEONARCHITECTRUNTIME_API UGridFlowBuilder : public UDungeonBuilder {
    GENERATED_BODY()

public:
    virtual void BuildDungeonImpl(UWorld* World) override;
    virtual void EmitDungeonMarkers_Implementation() override;
    virtual void DrawDebugData(UWorld* InWorld, bool bPersistant = false, float lifeTime = -1.0f) override;
    virtual bool SupportsBackgroundTask() const override { return false; }
    virtual TSubclassOf<UDungeonModel> GetModelClass() override;
    virtual TSubclassOf<UDungeonConfig> GetConfigClass() override;
    virtual TSubclassOf<UDungeonToolData> GetToolDataClass() override;
    virtual TSubclassOf<UDungeonQuery> GetQueryClass() override;
    virtual bool ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform,
                                  FQuat& OutRotationOffset) override;
    virtual void GetDefaultMarkerNames(TArray<FString>& OutMarkerNames) override;
    virtual bool PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics,
                               const FPropSocket& socket) override;
    virtual FTransform PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics,
                                     const FPropSocket& socket) override;
    virtual void ProcessThemeItemUserData(TSharedPtr<IDungeonMarkerUserData> UserData, AActor* SpawnedActor) override;


protected:
    bool ExecuteGraph();
    void EmitMarkerAt(const FVector& WorldLocation, const FString& MarkerName, const FQuat& Rotation,
                      TSharedPtr<class IDungeonMarkerUserData> InUserData = nullptr);
    void EmitMarkerAt(const FVector& WorldLocation, const FString& MarkerName, float Angle,
                      TSharedPtr<class IDungeonMarkerUserData> InUserData = nullptr);

    void EmitEdgeMarker(const FGridFlowTilemapEdge& Edge, const FVector& TileCoord,
                        const FVector& GridSize, UGridFlowTilemap* Tilemap, const TMap<FGuid, const UFlowGraphItem*>& Items);
private:
    UGridFlowModel* GridFlowModel = nullptr;
    UGridFlowConfig* GridFlowConfig = nullptr;
    UGridFlowQuery* GridFlowQuery = nullptr;
    FDAAttributeList AttributeList;
};


class DUNGEONARCHITECTRUNTIME_API FGridFlowBuilderMarkerUserData : public IDungeonMarkerUserData {
public:
    FGridFlowTilemapCoord TileCoord;
    bool bIsItem = false;
    TWeakObjectPtr<const UFlowGraphItem> Item;
};


class DUNGEONARCHITECTRUNTIME_API FGridFlowProcessDomainExtender : public IFlowProcessDomainExtender {
public:
    virtual void ExtendDomains(FFlowProcessor& InProcessor) override;

};

