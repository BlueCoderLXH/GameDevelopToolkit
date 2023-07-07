//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/Panels/Viewport/SFlowPreview3DViewport.h"

#include "Core/Dungeon.h"
#include "Core/Editors/FlowEditor/FlowEditor.h"
#include "Core/Editors/FlowEditor/Panels/Viewport/FlowPreview3DViewportClient.h"
#include "Core/Editors/FlowEditor/Panels/Viewport/SFlowPreview3DViewportToolbar.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "EditorViewportClient.h"
#include "Engine/TextureCube.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "SFlowPreview3DViewport"
DEFINE_LOG_CATEGORY_STATIC(LogGridFlowPreviewViewport, Log, All);

void SFlowPreview3DViewport::Construct(const FArguments& InArgs) {
    FlowAsset = InArgs._FlowAsset;
    FlowEditor = InArgs._FlowEditor;

    PreviewScene = MakeShareable(new FPreviewScene);
    SEditorViewport::Construct(SEditorViewport::FArguments());
    //ObjectToEdit->PreviewViewportProperties->PropertyChangeListener = SharedThis(this);

    UWorld* World = PreviewScene->GetWorld();

    PreviewScene->DirectionalLight->SetMobility(EComponentMobility::Movable);
    PreviewScene->DirectionalLight->CastShadows = true;
    PreviewScene->DirectionalLight->CastStaticShadows = true;
    PreviewScene->DirectionalLight->CastDynamicShadows = true;
    PreviewScene->DirectionalLight->SetIntensity(1.0f);

    UTextureCube* Cubemap = Cast<UTextureCube>(
        StaticLoadObject(UTextureCube::StaticClass(), nullptr, TEXT("/Engine/MapTemplates/daylight")));
    PreviewScene->SetSkyCubemap(Cubemap);
    PreviewScene->SetSkyBrightness(0.5f);
    PreviewScene->UpdateCaptureContents();

    BindCommands();
}


SFlowPreview3DViewport::~SFlowPreview3DViewport() {
    FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
    if (EditorViewportClient.IsValid()) {
        EditorViewportClient->Viewport = nullptr;
    }
}

void SFlowPreview3DViewport::AddReferencedObjects(FReferenceCollector& Collector) {
}


void SFlowPreview3DViewport::OnToggleDebugData() {
    UE_LOG(LogGridFlowPreviewViewport, Log, TEXT("Toggle debug data"));
}

void SFlowPreview3DViewport::OnDisplayDungeonProperties() {
    if (FlowEditor.IsValid() && PreviewDungeon.IsValid()) {
        TSharedPtr<FFlowEditorBase> FlowEditorPtr = FlowEditor.Pin();
        FlowEditorPtr->ShowObjectDetails(PreviewDungeon.Get());
    }
}

void SFlowPreview3DViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
                                      const float InDeltaTime) {
    SEditorViewport::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

}

EVisibility SFlowPreview3DViewport::GetToolbarVisibility() const {
    return EVisibility::Visible;
}

TSharedRef<FEditorViewportClient> SFlowPreview3DViewport::MakeEditorViewportClient() {
    EditorViewportClient = MakeShareable(
        new FFlowPreview3DViewportClient(*PreviewScene, SharedThis(this)));

    //EditorViewportClient->bSetListenerPosition = false;
    EditorViewportClient->SetRealtime(true);
    EditorViewportClient->VisibilityDelegate.BindSP(this, &SFlowPreview3DViewport::IsVisible);

    return EditorViewportClient.ToSharedRef();
}

EVisibility SFlowPreview3DViewport::OnGetViewportContentVisibility() const {
    return EVisibility::Visible;
}

void SFlowPreview3DViewport::BindCommands() {
    SEditorViewport::BindCommands();

    const FGridFlowEditorViewportCommands& ViewportActions = FGridFlowEditorViewportCommands::Get();
    CommandList->MapAction(
        ViewportActions.ToggleDebugData,
        FExecuteAction::CreateSP(this, &SFlowPreview3DViewport::OnToggleDebugData));

    CommandList->MapAction(
        ViewportActions.DisplayDungeonProperties,
        FExecuteAction::CreateSP(this, &SFlowPreview3DViewport::OnDisplayDungeonProperties));

}

void SFlowPreview3DViewport::OnFocusViewportToSelection() {
    SEditorViewport::OnFocusViewportToSelection();
}

TSharedPtr<SWidget> SFlowPreview3DViewport::MakeViewportToolbar() {
    // Build our toolbar level toolbar
    TSharedRef<SFlowPreview3DViewportToolbar> ToolBar =
        SNew(SFlowPreview3DViewportToolbar)
		.Viewport(SharedThis(this))
		.Visibility(this, &SFlowPreview3DViewport::GetToolbarVisibility)
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

bool SFlowPreview3DViewport::IsVisible() const {
    return ViewportWidget.IsValid() && (!ParentTab.IsValid() || ParentTab.Pin()->IsForeground());
}

void SFlowPreview3DViewport::SetPreviewDungeon(ADungeon* InDungeon) {
    if (PreviewDungeon.IsValid()) {
        PreviewDungeon->DestroyDungeon();
        PreviewDungeon->Destroy();
    }

    PreviewDungeon = InDungeon;
}

UWorld* SFlowPreview3DViewport::GetWorld() const {
    return PreviewScene->GetWorld();
}

#undef LOCTEXT_NAMESPACE

