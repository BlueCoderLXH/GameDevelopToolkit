//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstractBase.h"
#include "FlowTaskAbstract_SpawnItems.generated.h"

UENUM()
enum class EridFlowTask_SpawnItemsMethod : uint8 {
    RandomRange UMETA(DisplayName = "Random Range"),
    LinearDifficulty UMETA(DisplayName = "Linear Difficulty")
};

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UFlowTaskAbstract_SpawnItems : public UFlowTaskAbstractBase {
    GENERATED_BODY()
public:
    /**
        The list of paths to spawn the items on

        Variable Name: Paths
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    TArray<FString> Paths;

    /**
        The type of item to spawn

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    EFlowGraphItemType ItemType = EFlowGraphItemType::Enemy;

    /**
        The marker name for this item. Create a marker in the theme file with this name and add you assets there

        Variable Name: MarkerName
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    FString MarkerName;

    /**
        If ItemType is set to custom, customize the visuals of the items in the graph preview

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    FGridFlowItemCustomInfo CustomItemInfo;

    /**
        Minimum number of items to spawn

        Variable Name: MinCount
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    int32 MinCount = 1;

    /**
        Maximum number of items to spawn

        Variable Name: MaxCount
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    int32 MaxCount = 4;

    /**
        The method to use to control the spawn

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    EridFlowTask_SpawnItemsMethod SpawnMethod = EridFlowTask_SpawnItemsMethod::LinearDifficulty;

    /**
        Control the random variance

        Variable Name: SpawnDistributionVariance
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    float SpawnDistributionVariance = 0.2f;

    /**
        Each node in the path has a difficulty, with the first node having a difficulty of 0
        and the last node 1 and everything in between
        Start spawning only on nodes having difficulty greater than this value

        Variable Name: MinSpawnDifficulty
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    float MinSpawnDifficulty = 0.0f;

    /**
        The probability of spawning in this node.  It might spawn as usual or not spawn anything at all depending
        on this value. 0 = 0% chance of spawning.  1 = 100% chance of spawning

        Variable Name: SpawnProbability
    */
    UPROPERTY(EditAnywhere, Category = "Spawn Items")
    float SpawnProbability = 1.0f;

    /**
        Debug: Show the difficulty of the nodes in this path

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugShowDifficulty = false;

    /**
        Debug: The color of the debug data item

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Debug")
    FLinearColor DifficultyInfoColor = FLinearColor(0, 0, 0.5f);

public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;

private:
    int32 GetSpawnCount(float Weight, const FRandomStream& Random);
};

