//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_CreateElevations.h"

#include "Core/Utils/Attributes.h"
#include "Core/Utils/Noise/Noise.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

void UGridFlowTaskTilemap_CreateElevations::Execute(const FFlowExecutionInput& Input,
                                                    const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
    if (Input.IncomingNodeOutputs.Num() == 0) {
        Output.ErrorMessage = "Missing Input";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }
    if (Input.IncomingNodeOutputs.Num() > 1) {
        Output.ErrorMessage = "Only one input allowed";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    Output.State = Input.IncomingNodeOutputs[0].State->Clone();
    UGridFlowTilemap* Tilemap = Output.State->GetState<UGridFlowTilemap>(UGridFlowTilemap::StateTypeID);
    
    if (!Tilemap) {
        Output.ErrorMessage = "Invalid Input Tilemap";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    const FRandomStream& Random = *Input.Random;
    FGradientNoiseTable NoiseTable;
    NoiseTable.Init(128, Random);

    for (int y = 0; y < Tilemap->GetHeight(); y++) {
        for (int x = 0; x < Tilemap->GetWidth(); x++) {
            FGridFlowTilemapCell& Cell = Tilemap->Get(x, y);
            float cellHeight = 0;
            if (Cell.CellType == EGridFlowTilemapCellType::Empty) {
                FVector2D Position = FVector2D(x, y) * NoiseFrequency;
                float Noise = NoiseTable.GetFbmNoise(Position, NoiseOctaves);
                if (NoiseValuePower > 1e-6f) {
                    Noise = FMath::Pow(Noise, NoiseValuePower);
                }
                Noise = FMath::FloorToInt(Noise * NumSteps) / static_cast<float>(NumSteps);
                cellHeight = MinHeight + Noise * (MaxHeight - MinHeight);
            }

            Cell.CellType = EGridFlowTilemapCellType::Custom;
            Cell.CustomCellInfo.MarkerName = MarkerName;
            Cell.Height = cellHeight;
            FLinearColor color = (Cell.Height <= SeaLevel) ? SeaColor : LandColor;
            FLinearColor minColor = color * MinColorMultiplier;
            float colorBrightness = 1.0f;
            if (FMath::Abs(MaxHeight - MinHeight) > 1e-6f) {
                colorBrightness = (Cell.Height - MinHeight) / (MaxHeight - MinHeight);
            }
            Cell.CustomCellInfo.DefaultColor = FMath::Lerp(minColor, color, colorBrightness);
        }
    }

    Output.ExecutionResult = EFlowTaskExecutionResult::Success;
}

bool UGridFlowTaskTilemap_CreateElevations::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    FLOWTASKATTR_GET_INT(NoiseOctaves);
    FLOWTASKATTR_GET_FLOAT(NoiseFrequency);
    FLOWTASKATTR_GET_FLOAT(NoiseValuePower);
    FLOWTASKATTR_GET_INT(NumSteps);
    FLOWTASKATTR_GET_STRING(MarkerName);
    FLOWTASKATTR_GET_FLOAT(MinHeight);
    FLOWTASKATTR_GET_FLOAT(MaxHeight);
    FLOWTASKATTR_GET_FLOAT(SeaLevel);

    return false;
}

bool UGridFlowTaskTilemap_CreateElevations::SetParameter(const FString& InParameterName,
                                                         const FDAAttribute& InValue) {
    FLOWTASKATTR_SET_INT(NoiseOctaves);
    FLOWTASKATTR_SET_FLOAT(NoiseFrequency);
    FLOWTASKATTR_SET_FLOAT(NoiseValuePower);
    FLOWTASKATTR_SET_INT(NumSteps);
    FLOWTASKATTR_SET_STRING(MarkerName);
    FLOWTASKATTR_SET_FLOAT(MinHeight);
    FLOWTASKATTR_SET_FLOAT(MaxHeight);
    FLOWTASKATTR_SET_FLOAT(SeaLevel);

    return false;
}

bool UGridFlowTaskTilemap_CreateElevations::SetParameterSerialized(const FString& InParameterName,
                                                                   const FString& InSerializedText) {
    FLOWTASKATTR_SET_PARSE_INT(NoiseOctaves);
    FLOWTASKATTR_SET_PARSE_FLOAT(NoiseFrequency);
    FLOWTASKATTR_SET_PARSE_FLOAT(NoiseValuePower);
    FLOWTASKATTR_SET_PARSE_INT(NumSteps);
    FLOWTASKATTR_SET_PARSE_STRING(MarkerName);
    FLOWTASKATTR_SET_PARSE_FLOAT(MinHeight);
    FLOWTASKATTR_SET_PARSE_FLOAT(MaxHeight);
    FLOWTASKATTR_SET_PARSE_FLOAT(SeaLevel);

    return false;
}

