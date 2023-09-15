//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapConnectionEditor/Preview3D/SSnapConnectionPreview3DViewport.h"

#include "Core/Editors/SnapConnectionEditor/Preview3D/SSnapConnectionPreview3DViewportToolbar.h"
#include "Core/Editors/SnapConnectionEditor/Preview3D/SnapConnectionPreview3DViewportClient.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"

#include "AdvancedPreviewScene.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "EditorViewportClient.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "SSnapConnectionPreview3DViewport"


void SSnapConnectionPreview3DViewport::Construct(const FArguments& InArgs) {
    {
        FAdvancedPreviewScene::ConstructionValues CVS;
        CVS.bCreatePhysicsScene = false;
        CVS.LightBrightness = 3;
        CVS.SkyBrightness = 1;
        PreviewScene = MakeShareable(new FAdvancedPreviewScene(CVS));
        PreviewScene->SetFloorVisibility(false);

        // Make sure the floor is not visible even if enabled from the properties tab
        PreviewScene->SetFloorOffset(-100000);
    }

    SEditorViewport::Construct(SEditorViewport::FArguments());
    //ObjectToEdit->PreviewViewportProperties->PropertyChangeListener = SharedThis(this);

    UWorld* World = PreviewScene->GetWorld();

    AtmosphericFog = NewObject<USkyAtmosphereComponent>();
    PreviewScene->AddComponent(AtmosphericFog, FTransform::Identity);

    PreviewScene->DirectionalLight->SetMobility(EComponentMobility::Movable);
    PreviewScene->DirectionalLight->CastShadows = true;
    PreviewScene->DirectionalLight->CastStaticShadows = true;
    PreviewScene->DirectionalLight->CastDynamicShadows = true;
    PreviewScene->SetSkyBrightness(1.0f);

    SpawnPreviewActor();
}


SSnapConnectionPreview3DViewport::~SSnapConnectionPreview3DViewport() {
    FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
    if (EditorViewportClient.IsValid()) {
        EditorViewportClient->Viewport = nullptr;
    }
}

void SSnapConnectionPreview3DViewport::AddReferencedObjects(FReferenceCollector& Collector) {
    Collector.AddReferencedObject(AtmosphericFog);
    Collector.AddReferencedObject(PreviewActor);
}


void SSnapConnectionPreview3DViewport::OnToggleDebugData() {

}

void SSnapConnectionPreview3DViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
                                               const float InDeltaTime) {
    SEditorViewport::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

}

EVisibility SSnapConnectionPreview3DViewport::GetToolbarVisibility() const {
    return EVisibility::Visible;
}

TSharedRef<FEditorViewportClient> SSnapConnectionPreview3DViewport::MakeEditorViewportClient() {
    EditorViewportClient = MakeShareable(new FSnapConnectionPreview3DViewportClient(*PreviewScene));

    EditorViewportClient->bSetListenerPosition = false;
    EditorViewportClient->SetRealtime(true); // TODO: Check if real-time is needed
    EditorViewportClient->VisibilityDelegate.BindSP(this, &SSnapConnectionPreview3DViewport::IsVisible);

    return EditorViewportClient.ToSharedRef();
}

EVisibility SSnapConnectionPreview3DViewport::OnGetViewportContentVisibility() const {
    return EVisibility::Visible;
}

void SSnapConnectionPreview3DViewport::BindCommands() {
    SEditorViewport::BindCommands();

    const FSnapConnectionEditorViewportCommands& ViewportActions = FSnapConnectionEditorViewportCommands::Get();
    CommandList->MapAction(
        ViewportActions.ToggleDebugData,
        FExecuteAction::CreateSP(this, &SSnapConnectionPreview3DViewport::OnToggleDebugData));

}

void SSnapConnectionPreview3DViewport::OnFocusViewportToSelection() {
    SEditorViewport::OnFocusViewportToSelection();
}

TSharedPtr<SWidget> SSnapConnectionPreview3DViewport::MakeViewportToolbar() {
    // Build our toolbar level toolbar
    TSharedRef<SSnapConnectionPreview3DViewportToolbar> ToolBar =
        SNew(SSnapConnectionPreview3DViewportToolbar)
		.Viewport(SharedThis(this))
		.Visibility(this, &SSnapConnectionPreview3DViewport::GetToolbarVisibility)
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute());

    return
        SNew(SVerticalBox)
        .Visibility(EVisibility::SelfHitTestInvisible)
        + SVerticalBox::Slot()
          .AutoHeight()
          .VAlign(VAlign_Top)
        [
            ToolBar
        ];
}

bool SSnapConnectionPreview3DViewport::IsVisible() const {
    return ViewportWidget.IsValid() && (!ParentTab.IsValid() || ParentTab.Pin()->IsForeground());
}

void SSnapConnectionPreview3DViewport::SpawnPreviewActor() {
    if (PreviewActor) {
        PreviewActor->Destroy();
    }
    UWorld* World = PreviewScene->GetWorld();
    PreviewActor = World->SpawnActor<ASnapConnectionActor>(FVector::ZeroVector, FQuat::Identity.Rotator());
}


USnapConnectionComponent* SSnapConnectionPreview3DViewport::GetConnectionComponent() const {
    return PreviewActor->ConnectionComponent;
}

ASnapConnectionActor* SSnapConnectionPreview3DViewport::GetConnectionActor() const {
    return PreviewActor;
}

UWorld* SSnapConnectionPreview3DViewport::GetWorld() const {
    return PreviewScene->GetWorld();
}

#undef LOCTEXT_NAMESPACE

