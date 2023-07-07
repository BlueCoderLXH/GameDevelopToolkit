//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/Viewport/SnapMapPreviewScene.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/LineBatchComponent.h"
#include "Engine/Engine.h"

FSnapMapEditorPreviewScene::FSnapMapEditorPreviewScene(ConstructionValues CVS) : FAdvancedPreviewScene(CVS) {
    bForceAllUsedMipsResident = CVS.bForceMipsResident;
    PreviewWorld = NewObject<UWorld>();
    PreviewWorld->WorldType = EWorldType::EditorPreview;
    if (CVS.bTransactional) {
        PreviewWorld->SetFlags(RF_Transactional);
    }

    FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::EditorPreview);
    WorldContext.SetCurrentWorld(PreviewWorld);

    PreviewWorld->InitializeNewWorld(UWorld::InitializationValues()
                                     .AllowAudioPlayback(CVS.bAllowAudioPlayback)
                                     .CreatePhysicsScene(CVS.bCreatePhysicsScene)
                                     //.RequiresHitProxies(bCreateHitProxy)
                                     .CreateNavigation(false)
                                     .CreateAISystem(false)
                                     .ShouldSimulatePhysics(CVS.bShouldSimulatePhysics)
                                     .SetTransactional(CVS.bTransactional));
    PreviewWorld->InitializeActorsForPlay(FURL());

    //GetScene()->UpdateDynamicSkyLight(FLinearColor::White * CVS.SkyBrightness, FLinearColor::Black);
    //SetSkyBrightness(CVS.SkyBrightness);

    DirectionalLight = NewObject<UDirectionalLightComponent>(GetTransientPackage());
    DirectionalLight->Intensity = CVS.LightBrightness;
    DirectionalLight->LightColor = FColor::White;
    AddComponent(DirectionalLight, FTransform(CVS.LightRotation));

    LineBatcher = NewObject<ULineBatchComponent>(GetTransientPackage());
    AddComponent(LineBatcher, FTransform::Identity);
}

