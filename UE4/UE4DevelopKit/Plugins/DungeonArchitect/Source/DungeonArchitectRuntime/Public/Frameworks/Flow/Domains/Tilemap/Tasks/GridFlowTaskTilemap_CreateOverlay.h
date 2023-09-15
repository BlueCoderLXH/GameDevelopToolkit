//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/Noise/Noise.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemapBase.h"
#include "GridFlowTaskTilemap_CreateOverlay.generated.h"

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FGridFlowExecNodeTilemapOverlayNoiseSettings {
    GENERATED_BODY()

    /**
        Number of octaves to apply on the Perlin noise

        Variable Name: NoiseOctaves
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    int32 NoiseOctaves = 4;

    /**
        Frequency of the Noise

        Variable Name: NoiseFrequency
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    float NoiseFrequency = 0.15f;

    /**

        Variable Name: NoiseValuePower
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    float NoiseValuePower = 0;

    /**
        Min Noise Value. Higher generated noise values override other overlay noise values lower than this

        Variable Name: NoiseMinValue
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    float NoiseMinValue = 0;

    /**
        Max Noise Value. Higher generated noise values override other overlay noise values lower than this

        Variable Name: NoiseMaxValue
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    float NoiseMaxValue = 1.0f;

    /**
        If the generated noise value is greater than this value, the overlay is created

        Variable Name: NoiseThreshold
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    float NoiseThreshold = 0.5f;

    /**
        How far away should the overlays be created from the main path

        Variable Name: MinDistFromMainPath
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    int32 MinDistFromMainPath = 2;
};

struct FGridFlowTilemapCell;

UCLASS(Meta = (TilemapTask, Title = "Create Overlay", Tooltip = "Create Overlay", MenuPriority = 2300))
class DUNGEONARCHITECTRUNTIME_API UGridFlowTaskTilemap_CreateOverlay : public UGridFlowTaskTilemapBase {
    GENERATED_BODY()
public:
    /**
        The name of the marker to emit. Create a marker node with this name in the theme file and 
        add your overlay asset there

        Variable Name: MarkerName
    */
    UPROPERTY(EditAnywhere, Category = "Create Overlay")
    FString MarkerName = "Tree";

    /**
        The visualization color of the overlay in the preview viewports

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Create Overlay")
    FLinearColor Color = FLinearColor::Green;

    /**
        Indicates if this overlay blocks the walkable area.  (E.g. trees/rocks block, while grass doesn't)
        This test is used to make sure items are not placed behind rocks and trees where the player might not
        be able to get to

        Variable Name: bOverlayBlocksTile
    */
    UPROPERTY(EditAnywhere, Category = "Create Overlay")
    bool bOverlayBlocksTile = true;

    UPROPERTY(EditAnywhere, Category = "Create Overlay")
    FGridFlowExecNodeTilemapOverlayNoiseSettings NoiseSettings;

    UPROPERTY(EditAnywhere, Category = "Create Overlay")
    FGridFlowTilemapCellOverlayMergeConfig MergeConfig;

public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;

private:
    bool GenerateOverlayValue(FGridFlowTilemapCell& Cell, const FGridFlowTilemapCell& IncomingCell,
                              const FGradientNoiseTable& NoiseTable, float& OutValue);
};

