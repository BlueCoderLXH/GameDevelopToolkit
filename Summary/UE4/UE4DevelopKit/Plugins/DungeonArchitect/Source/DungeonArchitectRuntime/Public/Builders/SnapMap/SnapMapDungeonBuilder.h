//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/SnapMap/SnapMapDungeonConfig.h"
#include "Core/DungeonBuilder.h"
#include "Core/DungeonModel.h"
#include "Frameworks/LevelStreaming/DungeonLevelStreamingModel.h"
#include "Frameworks/Snap/Lib/Streaming/SnapStreaming.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"
#include "SnapMapDungeonBuilder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SnapMapDungeonBuilderLog, Log, All);

class ADungeon;
class USnapMapDungeonConfig;
class USnapConnectionComponent;
class USnapMapDungeonModel;
class FDungeonSceneProvider;
class ULevelStreamingDynamic;
class ASnapConnectionActor;
class ANavMeshBoundsVolume;
class UDungeonLevelStreamingModel;
class UDungeonStreamingChunk;

namespace SnapLib
{
	class FDiagnostics;
}

namespace SnapLib
{
	typedef TSharedPtr<struct FModuleNode> FModuleNodePtr;
	typedef TSharedPtr<struct FModuleDoor> FModuleDoorPtr;
}


/**
* The snap map builder
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API USnapMapDungeonBuilder : public UDungeonBuilder
{
	GENERATED_UCLASS_BODY()
public:
	virtual void BuildNonThemedDungeonImpl(UWorld* World, TSharedPtr<FDungeonSceneProvider> SceneProvider) override;
	virtual void DestroyNonThemedDungeonImpl(UWorld* World) override;

	virtual void DrawDebugData(UWorld* InWorld, bool bPersistent = false, float LifeTime = -1.0f) override;
	virtual bool SupportsBackgroundTask() const override { return false; }
	virtual TSubclassOf<UDungeonModel> GetModelClass() override;
	virtual TSubclassOf<UDungeonConfig> GetConfigClass() override;
	virtual TSubclassOf<UDungeonToolData> GetToolDataClass() override;
	virtual TSubclassOf<UDungeonQuery> GetQueryClass() override;
	virtual bool ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform,
	                                      FQuat& OutRotationOffset) override;
	virtual bool SupportsProperty(const FName& PropertyName) const override;
	virtual bool SupportsTheming() const override { return false; }
	virtual TSharedPtr<class FDungeonSceneProvider> CreateSceneProvider(UDungeonConfig* Config, ADungeon* pDungeon,
	                                                                    UWorld* World) override;
	virtual bool CanBuildDungeon(FString& OutMessage) override;
	virtual bool SupportsLevelStreaming() const override { return true; }

	void SetDiagnostics(TSharedPtr<SnapLib::FDiagnostics> InDiagnostics);

	UFUNCTION(BlueprintCallable, Category = Dungeon)
	void BuildPreviewSnapLayout();

	static void GetSnapConnectionActors(ULevel* ModuleLevel, TArray<ASnapConnectionActor*>& OutConnectionActors);

protected:
	virtual bool PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics,
	                                   const FPropSocket& socket) override;
	virtual FTransform PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics,
	                                         const FPropSocket& socket) override;

	SnapLib::FModuleNodePtr GenerateModuleNodeGraph(int32 InSeed) const;

protected:
	TWeakObjectPtr<USnapMapDungeonModel> SnapMapModel;
	TWeakObjectPtr<USnapMapDungeonConfig> SnapMapConfig;
	TSharedPtr<class FSnapMapStreamingChunkHandler> LevelStreamHandler;

	// Optional diagnostics for the dungeon flow editor. Will not be used in standalone builds
	TSharedPtr<SnapLib::FDiagnostics> Diagnostics;
};

class FSnapMapStreamingChunkHandler : public FSnapStreamingChunkHandlerBase
{
public:
	FSnapMapStreamingChunkHandler(UWorld* InWorld, USnapMapDungeonModel* InSnapMapModel,
	                              UDungeonLevelStreamingModel* InLevelStreamingModel);
	virtual TArray<struct FSnapConnectionInstance>* GetConnections() const override;
	virtual UDungeonLevelStreamingModel* GetLevelStreamingModel() const override;
	virtual UWorld* GetWorld() const override;

public:
	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<USnapMapDungeonModel> SnapMapModel;
	TWeakObjectPtr<UDungeonLevelStreamingModel> LevelStreamingModel;
};
