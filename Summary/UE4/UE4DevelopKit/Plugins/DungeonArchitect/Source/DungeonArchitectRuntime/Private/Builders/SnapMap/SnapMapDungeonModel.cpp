//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SnapMap/SnapMapDungeonModel.h"

#include "Core/DungeonLayoutData.h"
#include "Frameworks/Snap/Lib/Serialization/ConnectionSerialization.h"

USnapMapDungeonModel::USnapMapDungeonModel(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
}

void USnapMapDungeonModel::Reset() {
    Cleanup();
    Connections.Reset();
    ModuleInstances.Reset();
    MissionGraph = nullptr;
}

void USnapMapDungeonModel::GenerateLayoutData(FDungeonLayoutData& OutLayout) {
    FBox WorldBounds(ForceInit);
    for (const FSnapMapModuleInstanceSerializedData& ModuleInstance : ModuleInstances) {
        FBox ModuleWorldBounds = ModuleInstance.ModuleBounds.TransformBy(ModuleInstance.WorldTransform);
        WorldBounds += ModuleWorldBounds;
    }
    FVector Offset = -WorldBounds.Min;
    FVector WorldSize = WorldBounds.GetSize();
    float Scale = 1.0f / FMath::Max(WorldSize.X, WorldSize.Y);
    FVector Scale3D(Scale);
    Offset *= Scale3D;

    FTransform WorldToScreen;
    {
        const float Padding = 0.1f;
        FTransform NormalizeTransform = FTransform(FRotator::ZeroRotator, Offset, Scale3D);
        FTransform PaddingTransform = FTransform(FRotator::ZeroRotator, FVector(Padding), FVector(1.0f - Padding * 2));
        WorldToScreen = NormalizeTransform * PaddingTransform;
    }

    for (const FSnapMapModuleInstanceSerializedData& ModuleInstance : ModuleInstances) {
        FBox Bounds = ModuleInstance.ModuleBounds
                                    .TransformBy(ModuleInstance.WorldTransform)
                                    .TransformBy(WorldToScreen);

        FVector Min = Bounds.Min;
        FVector Size = Bounds.GetSize();

        FVector2D Min2D(Min.X, Min.Y);
        FVector2D Size2D(Size.X, Size.Y);
        OutLayout.AddQuadItem(Min2D, Size2D);
    }

    // Add the doors
    for (const FSnapConnectionInstance& Connection : Connections) {
        FVector A(0, -200, 0); // TODO: Parameterize the size
        FVector B(0, 200, 0);
        FVector WorldA = Connection.WorldTransform.TransformPosition(A);
        FVector WorldB = Connection.WorldTransform.TransformPosition(B);

        FVector NormA = WorldToScreen.TransformPosition(WorldA);
        FVector NormB = WorldToScreen.TransformPosition(WorldB);

        FDungeonLayoutDataDoorItem Door;
        Door.Outline.Add(FVector2D(NormA.X, NormA.Y));
        Door.Outline.Add(FVector2D(NormB.X, NormB.Y));

        OutLayout.Doors.Add(Door);
    }

    // Add Points of interest
    for (const FSnapMapModuleInstanceSerializedData& ModuleInstance : ModuleInstances) {
        FBox Bounds = ModuleInstance.ModuleBounds
                                    .TransformBy(ModuleInstance.WorldTransform)
                                    .TransformBy(WorldToScreen);

        FVector Center3D = Bounds.GetCenter();
        FDungeonLayoutDataPointOfInterest PointOfInterest;
        PointOfInterest.Location = FVector2D(Center3D.X, Center3D.Y);
        PointOfInterest.Id = ModuleInstance.Category;
        PointOfInterest.Caption = ModuleInstance.Category;

        OutLayout.PointsOfInterest.Add(PointOfInterest);
    }

    OutLayout.WorldToScreen = WorldToScreen;
}

bool USnapMapDungeonModel::SearchModuleInstance(const FGuid& InNodeId,
                                                FSnapMapModuleInstanceSerializedData& OutModuleData) {
    for (const FSnapMapModuleInstanceSerializedData& ModuleData : ModuleInstances) {
        if (ModuleData.ModuleInstanceId == InNodeId) {
            OutModuleData = ModuleData;
            return true;
        }
    }
    return false;
}

