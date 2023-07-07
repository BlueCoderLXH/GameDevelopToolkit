//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "DungeonLevelStreamer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLevelStreamer, Log, All);

class UDungeonStreamingChunk;

UENUM(BlueprintType)
enum class EDungeonLevelStreamingLoadingStrategy : uint8
{
	PreloadEverything UMETA(DisplayName = "Preload Everything"),
	LoadOnDemand UMETA(DisplayName = "Load On Demand"),
	OnlyVisibleLoaded UMETA(DisplayName = "Only Visible Loaded (unloaded hidden)"),
};

UENUM(BlueprintType)
enum class EDungeonLevelStreamingStrategy : uint8
{
	LayoutDepth UMETA(DisplayName = "Layout Depth"),
	Distance UMETA(DisplayName = "Distance")
};

UENUM(BlueprintType)
enum class EDungeonLevelStreamLoadMethod : uint8
{
	LoadOnDemand UMETA(DisplayName = "Load On Demand"),
	LoadEverythingInMemory UMETA(DisplayName = "Load Everything In Memory")
};

UENUM(BlueprintType)
enum class EDungeonLevelStreamUnloadMethod : uint8
{
	KeepHiddenChunksInMemory UMETA(DisplayName = "Keep Hidden Chunks In Memory"),
	UnloadHiddenChunks UMETA(DisplayName = "Unload Hidden Chunks")
};

UENUM(Blueprintable)
enum class EDungeonLevelStreamChunkSelection : uint8
{
	PlayerLocations UMETA(DisplayName = "Player Locations"),
	SpawnRoomLocations UMETA(DisplayName = "Spawn Room Locations"),
};


USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FDungeonLevelStreamingConfig
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Streaming")
	bool bEnabledLevelStreaming = true;

	/**
	 * Define what gets initially streamed in first when the game starts
	 * PlayerLocation - The chunks closest to the player get streamed in first
	 * SpawnRoom - The chunks tagged as spawn rooms get streamed in first
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Streaming", Meta=(EditCondition = "bEnabledLevelStreaming"))
	EDungeonLevelStreamChunkSelection InitialLoadLocation = EDungeonLevelStreamChunkSelection::SpawnRoomLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Streaming", Meta=(EditCondition = "bEnabledLevelStreaming"))
	EDungeonLevelStreamingStrategy StreamingStrategy = EDungeonLevelStreamingStrategy::LayoutDepth;

	/**
	 *  The nearby streamed chunks will be decided based on the connected rooms (from the layout graph). This is great for FPS games
	 *  This requires the Streaming Strategy to be set to LayoutDepth
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Streaming",
		Meta=(EditCondition = "bEnabledLevelStreaming && StreamingStrategy == EDungeonLevelStreamingStrategy::LayoutDepth"))
	int32 VisibilityRoomDepth = 1;

	/**
	 * The nearby streamed chunks will be decided based on the distance from the chunk the player is on.  This is great for side-scrollers and top-down games
	 * The bounds of the current chunk will be expanded by the specified amount and any nearby chunk overlapping with it will be shown
	 * This requires the Streaming Strategy to be set to LayoutDepth
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Streaming",
		Meta=(EditCondition = "bEnabledLevelStreaming && StreamingStrategy == EDungeonLevelStreamingStrategy::Distance"))
	FVector VisibilityDistance = FVector(3000, 3000, 3000);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Streaming", Meta = (EditCondition = "bEnabledLevelStreaming"))
	EDungeonLevelStreamLoadMethod LoadMethod = EDungeonLevelStreamLoadMethod::LoadOnDemand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Streaming", Meta = (EditCondition = "bEnabledLevelStreaming"))
	EDungeonLevelStreamUnloadMethod UnloadMethod = EDungeonLevelStreamUnloadMethod::UnloadHiddenChunks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Streaming", Meta = (EditCondition = "bEnabledLevelStreaming"))
	bool bProcessStreamingNavigation = false;
};

class UDungeonLevelStreamingModel;

class DUNGEONARCHITECTRUNTIME_API IDungeonLevelStreamerVisibilityStrategy
{
public:
	virtual ~IDungeonLevelStreamerVisibilityStrategy()
	{
	}

	virtual void GetVisibleChunks(UDungeonStreamingChunk* StartChunk, const TArray<UDungeonStreamingChunk*>& AllChunks,
	                              TArray<UDungeonStreamingChunk*>& OutVisibleChunks) const = 0;
};

class DUNGEONARCHITECTRUNTIME_API FDungeonLevelStreamer
{
public:
	static void Process(UWorld* InWorld, const FDungeonLevelStreamingConfig& Config, UDungeonLevelStreamingModel* InModel);

private:
	static void GetVisibleChunks(const FDungeonLevelStreamingConfig& Config, UDungeonStreamingChunk* StartChunk,
	                             const TArray<UDungeonStreamingChunk*>& AllChunks,
	                             TArray<UDungeonStreamingChunk*>& OutVisibleChunks);
	static bool GetNearestChunk(const TArray<UDungeonStreamingChunk*>& Chunks, const FVector& ViewLocation, UDungeonStreamingChunk*& OutNearestChunk);
	static bool ChunksLoadedAndVisible(const TArray<UDungeonStreamingChunk*>& InChunks);
	static void GetPlayerLocations(UWorld* World, TArray<FVector>& OutPlayerLocations);
};
