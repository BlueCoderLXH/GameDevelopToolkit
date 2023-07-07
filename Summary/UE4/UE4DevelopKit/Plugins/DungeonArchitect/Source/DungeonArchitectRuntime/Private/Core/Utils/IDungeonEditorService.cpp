//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/EditorService/IDungeonEditorService.h"

#include "Engine/LocalPlayer.h"

TSharedPtr<IDungeonEditorService> IDungeonEditorService::Instance;


TSharedPtr<IDungeonEditorService> IDungeonEditorService::Get() {
    return Instance;
}

void IDungeonEditorService::Set(TSharedPtr<IDungeonEditorService> InInstance) {
    Instance = InInstance;
}

bool IDungeonEditorService::GetLevelViewportCameraInfo(UWorld* World, FVector& CameraLocation, FRotator& CameraRotation) {
    ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();
    if (!LocalPlayer) {
        return false;
    }

    // Create a view family for the game viewport
    FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
        LocalPlayer->ViewportClient->Viewport,
        World->Scene,
        LocalPlayer->ViewportClient->EngineShowFlags)
        .SetRealtimeUpdate(false));

    // Calculate a view where the player is to update the streaming from the players start location
    FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, CameraLocation, CameraRotation, LocalPlayer->ViewportClient->Viewport);
    return SceneView != nullptr;
}

