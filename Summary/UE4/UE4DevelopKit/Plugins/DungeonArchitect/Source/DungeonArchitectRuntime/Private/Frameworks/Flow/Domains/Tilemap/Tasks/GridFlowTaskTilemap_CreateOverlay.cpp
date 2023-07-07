//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_CreateOverlay.h"

#include "Core/Utils/Attributes.h"
#include "Core/Utils/Noise/Noise.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

void UGridFlowTaskTilemap_CreateOverlay::Execute(const FFlowExecutionInput& Input,
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

    // TODO: Avoid cloning incoming tilemap as it is never used
    Output.State = Input.IncomingNodeOutputs[0].State->Clone();
    UGridFlowTilemap* Tilemap = Output.State->GetState<UGridFlowTilemap>(UGridFlowTilemap::StateTypeID);
    UGridFlowTilemap* IncomingTilemap = Input.IncomingNodeOutputs[0].State->GetState<UGridFlowTilemap>(UGridFlowTilemap::StateTypeID);
    if (!IncomingTilemap) {
        Output.ErrorMessage = "Invalid Input Tilemap";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    const int32 Width = IncomingTilemap->GetWidth();
    const int32 Height = IncomingTilemap->GetHeight();
    Tilemap->Initialize(Width, Height);

    const FRandomStream& Random = *Input.Random;
    FGradientNoiseTable NoiseTable;
    NoiseTable.Init(128, Random);
    NoiseSettings.MinDistFromMainPath = FMath::Max(1, NoiseSettings.MinDistFromMainPath);

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            FGridFlowTilemapCell& Cell = Tilemap->Get(x, y);
            const FGridFlowTilemapCell& IncomingCell = IncomingTilemap->Get(x, y);

            float OverlayValue = 0.0f;
            if (GenerateOverlayValue(Cell, IncomingCell, NoiseTable, OverlayValue)) {
                Cell.bHasOverlay = true;
                FGridFlowTilemapCellOverlay& Overlay = Cell.Overlay;
                Overlay.MarkerName = MarkerName;
                Overlay.Color = Color;
                Overlay.NoiseValue = OverlayValue;
                Overlay.MergeConfig = MergeConfig;
                Overlay.bTileBlockingOverlay = bOverlayBlocksTile;
            }
        }
    }

    Output.ExecutionResult = EFlowTaskExecutionResult::Success;
}

bool UGridFlowTaskTilemap_CreateOverlay::GenerateOverlayValue(FGridFlowTilemapCell& Cell,
                                                              const FGridFlowTilemapCell& IncomingCell,
                                                              const FGradientNoiseTable& NoiseTable, float& OutValue) {
    FIntPoint CellCoord = IncomingCell.TileCoord;
    FVector2D Position = FVector2D(CellCoord.X, CellCoord.Y) * NoiseSettings.NoiseFrequency;
    float N = NoiseTable.GetFbmNoise(Position, NoiseSettings.NoiseOctaves);
    if (NoiseSettings.NoiseValuePower > 0.0f) {
        N = FMath::Pow(N, NoiseSettings.NoiseValuePower);
    }

    N = NoiseSettings.NoiseMinValue + (NoiseSettings.NoiseMaxValue - NoiseSettings.NoiseMinValue) * N;

    if (N > NoiseSettings.NoiseThreshold) {
        int32 DistanceFromMainPath = IncomingCell.DistanceFromMainPath;
        float NoiseFactor = (N - NoiseSettings.NoiseThreshold) / (1.0f - NoiseSettings.NoiseThreshold);
        bool bInsertOverlay = (NoiseFactor * DistanceFromMainPath > NoiseSettings.MinDistFromMainPath);

        if (bInsertOverlay) {
            OutValue = N;
            return true;
        }
    }
    return false;
}


bool UGridFlowTaskTilemap_CreateOverlay::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    FLOWTASKATTR_GET_STRING(MarkerName)
    FLOWTASKATTR_GET_BOOL(bOverlayBlocksTile)

    // Handle Noise Settings
    __FLOWTASKATTR_GET_IMPL_EX(NoiseOctaves, NumberValue, NoiseSettings.NoiseOctaves)
    __FLOWTASKATTR_GET_IMPL_EX(NoiseFrequency, NumberValue, NoiseSettings.NoiseFrequency)
    __FLOWTASKATTR_GET_IMPL_EX(NoiseValuePower, NumberValue, NoiseSettings.NoiseValuePower)
    __FLOWTASKATTR_GET_IMPL_EX(NoiseMinValue, NumberValue, NoiseSettings.NoiseMinValue)
    __FLOWTASKATTR_GET_IMPL_EX(NoiseMaxValue, NumberValue, NoiseSettings.NoiseMaxValue)
    __FLOWTASKATTR_GET_IMPL_EX(NoiseThreshold, NumberValue, NoiseSettings.NoiseThreshold)
    __FLOWTASKATTR_GET_IMPL_EX(MinDistFromMainPath, NumberValue, NoiseSettings.MinDistFromMainPath)

    // Handle Merge Config
    __FLOWTASKATTR_GET_IMPL_EX(MinHeight, NumberValue, MergeConfig.MinHeight)
    __FLOWTASKATTR_GET_IMPL_EX(MaxHeight, NumberValue, MergeConfig.MaxHeight)
    __FLOWTASKATTR_GET_IMPL_EX(MarkerHeightOffsetForLayoutTiles, NumberValue,
                               MergeConfig.MarkerHeightOffsetForLayoutTiles)
    __FLOWTASKATTR_GET_IMPL_EX(MarkerHeightOffsetForNonLayoutTiles, NumberValue,
                               MergeConfig.MarkerHeightOffsetForNonLayoutTiles)
    __FLOWTASKATTR_GET_IMPL_EX(RemoveElevationMarker, BoolValue, MergeConfig.RemoveElevationMarker)

    return false;
}

bool UGridFlowTaskTilemap_CreateOverlay::SetParameter(const FString& InParameterName,
                                                      const FDAAttribute& InValue) {
    FLOWTASKATTR_SET_STRING(MarkerName)
    FLOWTASKATTR_SET_BOOL(bOverlayBlocksTile)

    // Handle Noise Settings
    __FLOWTASKATTR_SET_IMPL2(NoiseOctaves, NumberValue, NoiseSettings.NoiseOctaves)
    __FLOWTASKATTR_SET_IMPL2(NoiseFrequency, NumberValue, NoiseSettings.NoiseFrequency)
    __FLOWTASKATTR_SET_IMPL2(NoiseValuePower, NumberValue, NoiseSettings.NoiseValuePower)
    __FLOWTASKATTR_SET_IMPL2(NoiseMinValue, NumberValue, NoiseSettings.NoiseMinValue)
    __FLOWTASKATTR_SET_IMPL2(NoiseMaxValue, NumberValue, NoiseSettings.NoiseMaxValue)
    __FLOWTASKATTR_SET_IMPL2(NoiseThreshold, NumberValue, NoiseSettings.NoiseThreshold)
    __FLOWTASKATTR_SET_IMPL2(MinDistFromMainPath, NumberValue, NoiseSettings.MinDistFromMainPath)

    // Handle Merge Config
    __FLOWTASKATTR_SET_IMPL2(MinHeight, NumberValue, MergeConfig.MinHeight)
    __FLOWTASKATTR_SET_IMPL2(MaxHeight, NumberValue, MergeConfig.MaxHeight)
    __FLOWTASKATTR_SET_IMPL2(MarkerHeightOffsetForLayoutTiles, NumberValue,
                             MergeConfig.MarkerHeightOffsetForLayoutTiles)
    __FLOWTASKATTR_SET_IMPL2(MarkerHeightOffsetForNonLayoutTiles, NumberValue,
                             MergeConfig.MarkerHeightOffsetForNonLayoutTiles)
    __FLOWTASKATTR_SET_IMPL2(RemoveElevationMarker, BoolValue, MergeConfig.RemoveElevationMarker)

    return false;
}

bool UGridFlowTaskTilemap_CreateOverlay::SetParameterSerialized(const FString& InParameterName,
                                                                const FString& InSerializedText) {
    FLOWTASKATTR_SET_PARSE_STRING(MarkerName)
    FLOWTASKATTR_SET_PARSE_BOOL(bOverlayBlocksTile)

    // Handle Noise Settings
    FLOWTASKATTR_SET_PARSEEX_INT(NoiseOctaves, NoiseSettings.NoiseOctaves)
    FLOWTASKATTR_SET_PARSEEX_FLOAT(NoiseFrequency, NoiseSettings.NoiseFrequency)
    FLOWTASKATTR_SET_PARSEEX_FLOAT(NoiseValuePower, NoiseSettings.NoiseValuePower)
    FLOWTASKATTR_SET_PARSEEX_FLOAT(NoiseMinValue, NoiseSettings.NoiseMinValue)
    FLOWTASKATTR_SET_PARSEEX_FLOAT(NoiseMaxValue, NoiseSettings.NoiseMaxValue)
    FLOWTASKATTR_SET_PARSEEX_FLOAT(NoiseThreshold, NoiseSettings.NoiseThreshold)
    FLOWTASKATTR_SET_PARSEEX_INT(MinDistFromMainPath, NoiseSettings.MinDistFromMainPath)

    // Handle Merge Config
    FLOWTASKATTR_SET_PARSEEX_FLOAT(MinHeight, MergeConfig.MinHeight)
    FLOWTASKATTR_SET_PARSEEX_FLOAT(MaxHeight, MergeConfig.MaxHeight)
    FLOWTASKATTR_SET_PARSEEX_FLOAT(MarkerHeightOffsetForLayoutTiles, MergeConfig.MarkerHeightOffsetForLayoutTiles)
    FLOWTASKATTR_SET_PARSEEX_FLOAT(MarkerHeightOffsetForNonLayoutTiles, MergeConfig.MarkerHeightOffsetForNonLayoutTiles)
    FLOWTASKATTR_SET_PARSEEX_BOOL(RemoveElevationMarker, MergeConfig.RemoveElevationMarker)

    return false;
}

