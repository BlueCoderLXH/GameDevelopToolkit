//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/Viewport/SSnapMapEditorViewport.h"

#include "Core/Editors/SnapMapEditor/Viewport/SSnapMapEditorViewportToolbar.h"
#include "Core/Editors/SnapMapEditor/Viewport/SnapMapEditorViewportClient.h"

#include "AdvancedPreviewScene.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/LightComponentBase.h"
#include "EditorViewportClient.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "PreviewScene.h"
#include "Widgets/Docking/SDockTab.h"

void SSnapMapEditorViewport::Construct(const FArguments& InArgs) {
    DungeonFlowEditorPtr = InArgs._DungeonFlowEditor;
    ObjectToEdit = InArgs._ObjectToEdit;

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

    UWorld* World = PreviewScene->GetWorld();

    PreviewScene->DirectionalLight->SetMobility(EComponentMobility::Movable);
    PreviewScene->DirectionalLight->CastShadows = true;
    PreviewScene->DirectionalLight->CastStaticShadows = true;
    PreviewScene->DirectionalLight->CastDynamicShadows = true;
    PreviewScene->DirectionalLight->SetIntensity(3);
    PreviewScene->SetSkyBrightness(1.0f);
}

SSnapMapEditorViewport::~SSnapMapEditorViewport() {
    FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
    if (EditorViewportClient.IsValid()) {
        EditorViewportClient->Viewport = nullptr;
    }
}

void SSnapMapEditorViewport::AddReferencedObjects(FReferenceCollector& Collector) {
    Collector.AddReferencedObject(ObjectToEdit);
}


void SSnapMapEditorViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
                                      const float InDeltaTime) {
    SEditorViewport::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

}

EVisibility SSnapMapEditorViewport::GetToolbarVisibility() const {
    return EVisibility::Visible;
}

TSharedRef<FEditorViewportClient> SSnapMapEditorViewport::MakeEditorViewportClient() {
    EditorViewportClient = MakeShareable(new FSnapMapEditorViewportClient(*PreviewScene, SharedThis(this)));

    EditorViewportClient->bSetListenerPosition = false;
    EditorViewportClient->SetRealtime(true); // TODO: Check if real-time is needed
    EditorViewportClient->VisibilityDelegate.BindSP(this, &SSnapMapEditorViewport::IsVisible);

    return EditorViewportClient.ToSharedRef();
}

EVisibility SSnapMapEditorViewport::OnGetViewportContentVisibility() const {
    return EVisibility::Visible;
}

void SSnapMapEditorViewport::BindCommands() {
    SEditorViewport::BindCommands();

}

void SSnapMapEditorViewport::OnFocusViewportToSelection() {
    SEditorViewport::OnFocusViewportToSelection();
}

TSharedPtr<SWidget> SSnapMapEditorViewport::MakeViewportToolbar() {
    // Build our toolbar level toolbar
    TSharedRef<SViewportToolBar> ToolBar =
        SNew(SSnapMapEditorViewportToolBar)
		.Viewport(SharedThis(this))
		.Visibility(this, &SSnapMapEditorViewport::GetToolbarVisibility)
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

bool SSnapMapEditorViewport::IsVisible() const {
    return ViewportWidget.IsValid() && (!ParentTab.IsValid() || ParentTab.Pin()->IsForeground());
}

