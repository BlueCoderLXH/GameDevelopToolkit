//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemapBase.h"
#include "GridFlowTaskTilemap_CreateElevations.generated.h"

UCLASS(Meta = (TilemapTask, Title = "Create Elevation", Tooltip = "Create Elevation", MenuPriority = 2200))
class DUNGEONARCHITECTRUNTIME_API UGridFlowTaskTilemap_CreateElevations : public UGridFlowTaskTilemapBase {
    GENERATED_BODY()

public:
    /**
        Number of octaves to apply on the Perlin noise

        Variable Name: NoiseOctaves
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    int NoiseOctaves = 4;

    /**
        Frequency of the Noise

        Variable Name: NoiseFrequency
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    float NoiseFrequency = 0.01f;

    /**

        Variable Name: NoiseValuePower
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    float NoiseValuePower = 0;

    /**
        The Elevation noise field is divided into steps.  Increase this value for a smoother terrain

        Variable Name: NumSteps
    */
    UPROPERTY(EditAnywhere, Category = "Noise Settings")
    int NumSteps = 4;

    /**
        The marker name the emit on the elevation tile.   Create a marker node with this 
        name in the theme file and add your assets there

        Variable Name: MarkerName
    */
    UPROPERTY(EditAnywhere, Category = "Theme")
    FString MarkerName = "Elevation";

    /**
        The minimum height of the elevation. This value is in logical units and will be 
        multiplied with the Dungeon actor's GridSize config height. So a 
        logical height of -2 would be multiplied with 200 if the GridSize is (400, 400, 200) to 
        have the final value of -400 in unreal units

        Variable Name: MinHeight
    */
    UPROPERTY(EditAnywhere, Category = "Height Settings")
    float MinHeight = -20;

    /**
        The maximum height of the elevation. This value is in logical units and will be
        multiplied with the Dungeon actor's GridSize config height

        Variable Name: MaxHeight
    */
    UPROPERTY(EditAnywhere, Category = "Height Settings")
    float MaxHeight = -5;

    /**
        The sea level of the elevation. This value is in logical units and will be
        multiplied with the Dungeon actor's GridSize config height

        Variable Name: SeaLevel
    */
    UPROPERTY(EditAnywhere, Category = "Height Settings")
    float SeaLevel = -10;

    /**
        The color of the elevation tiles above the sea level.  Used only for visualization

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Color Settings")
    FLinearColor LandColor = FLinearColor(0.4f, 0.2f, 0);

    /**
        The color of the elevation tiles below the sea level.  Used only for visualization

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Color Settings")
    FLinearColor SeaColor = FLinearColor(0, 0, 0.4f);

    /**
        Adjust the color tint multiplier of the land/sea colors

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Color Settings")
    float MinColorMultiplier = 0.1f;

public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;
};

