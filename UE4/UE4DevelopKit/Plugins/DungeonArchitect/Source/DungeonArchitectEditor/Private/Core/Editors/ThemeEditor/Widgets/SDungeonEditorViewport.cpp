//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/SDungeonEditorViewport.h"

#include "Builders/Grid/GridDungeonBuilder.h"
#include "Core/Editors/ThemeEditor/Widgets/DungeonEditorViewportClient.h"
#include "Core/Editors/ThemeEditor/Widgets/SDungeonEditorViewportToolbar.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProvider.h"

#include "AdvancedPreviewScene.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/LightComponentBase.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "EditorViewportClient.h"
#include "EngineUtils.h"
#include "PreviewScene.h"
#include "Widgets/Docking/SDockTab.h"

void SDungeonEditorViewport::Construct(const FArguments& InArgs) {
    bRequestRebuildPreviewMesh = false;
    DungeonEditorPtr = InArgs._DungeonEditor;
    ObjectToEdit = InArgs._ObjectToEdit;
    bDrawDebugData = false;
    DungeonBuilder = nullptr;

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
    ObjectToEdit->PreviewViewportProperties->PropertyChangeListener = SharedThis(this);
    CreateDungeonBuilder(ObjectToEdit->PreviewViewportProperties->BuilderClass);

    UWorld* World = PreviewScene->GetWorld();
    DungeonBuilder->BuildDungeon(DungeonModel, ObjectToEdit->PreviewViewportProperties->DungeonConfig, DungeonQuery,
                                 World);

    Skylight = NewObject<USkyLightComponent>();
    //PreviewScene->AddComponent(Skylight, FTransform::Identity);

    AtmosphericFog = NewObject<USkyAtmosphereComponent>();
    PreviewScene->AddComponent(AtmosphericFog, FTransform::Identity);

    PreviewScene->DirectionalLight->SetMobility(EComponentMobility::Movable);
    PreviewScene->DirectionalLight->CastShadows = true;
    PreviewScene->DirectionalLight->CastStaticShadows = true;
    PreviewScene->DirectionalLight->CastDynamicShadows = true;
    PreviewScene->DirectionalLight->SetIntensity(3);
    PreviewScene->SetSkyBrightness(1.0f);

    UDungeonConfig* DungeonConfig = ObjectToEdit->PreviewViewportProperties->DungeonConfig;
    if (DungeonConfig) {
        ListenToConfigChanges(DungeonConfig);
    }
}


SDungeonEditorViewport::~SDungeonEditorViewport() {
    FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
    if (EditorViewportClient.IsValid()) {
        EditorViewportClient->Viewport = nullptr;
    }

    if (DungeonBuilder) {
        DungeonBuilder->RemoveFromRoot();
        DungeonBuilder = nullptr;
    }
    if (DungeonModel) {
        DungeonModel->RemoveFromRoot();
        DungeonModel = nullptr;
    }
    if (DungeonQuery) {
        DungeonQuery->RemoveFromRoot();
        DungeonQuery = nullptr;
    }
}

void SDungeonEditorViewport::AddReferencedObjects(FReferenceCollector& Collector) {
    Collector.AddReferencedObject(ObjectToEdit);
    Collector.AddReferencedObject(DungeonBuilder);
    Collector.AddReferencedObject(DungeonModel);
}

void SDungeonEditorViewport::RebuildMeshes() {
    bRequestRebuildPreviewMesh = true;
}

void SDungeonEditorViewport::PerformMeshRebuild() {
    if (!DungeonBuilder || !DungeonBuilder->SupportsTheming()) {
        return;
    }

    if (bForcePreviewRebuildLayout) {
        DungeonBuilder->BuildDungeon(DungeonModel, ObjectToEdit->PreviewViewportProperties->DungeonConfig, DungeonQuery,
                                     GetWorld());
    }
    else {
        // Since we do not rebuild the layout again for optimization in the theme editor, perform cleanup before reapplying the theme
        if (DungeonQuery && DungeonQuery->UserState) {
            DungeonQuery->UserState->ClearAllState();
        }
    }

    DungeonBuilder->EmitDungeonMarkers();

    // Emit custom markers in user-created blueprints
    //TArray<UDungeonMarkerEmitter*> Emitters = UDungeonModelHelper::CreateInstances<UDungeonMarkerEmitter>(
    //		ObjectToEdit->PreviewViewportProperties->MarkerEmitters);
    ensure(ObjectToEdit);
    TArray<UDungeonMarkerEmitter*> Emitters = ObjectToEdit->PreviewViewportProperties->MarkerEmitters;
    DungeonBuilder->EmitCustomMarkers(Emitters);

    UDungeonConfig* Config = ObjectToEdit->PreviewViewportProperties->DungeonConfig;
    SceneProvider = DungeonBuilder->CreateSceneProvider(Config, nullptr, PreviewScene->GetWorld());

    // While building the scene over multiple frames, start building the objects closer to the camera first
    FVector BuildPriorityLocation = EditorViewportClient->GetViewLocation();
    SceneProvider->SetBuildPriorityLocation(BuildPriorityLocation);

    CleanupModifiedNodeObjects();
    TArray<UDungeonThemeAsset*> Themes;
    Themes.Add(ObjectToEdit);

    TArray<FClusterThemeInfo> EmptyClusteredThemes; // No clustered themes shown in the theme editor viewport
    DungeonBuilder->ApplyDungeonTheme(Themes, EmptyClusteredThemes, SceneProvider, PreviewScene->GetWorld());
}


void SDungeonEditorViewport::OnShowPropertyDungeon() {
    ShowObjectProperties(ObjectToEdit->PreviewViewportProperties);
}

void SDungeonEditorViewport::OnShowPropertySkylight() {
    ShowObjectProperties(Skylight);
}

void SDungeonEditorViewport::OnShowPropertyDirectionalLight() {
    ShowObjectProperties(PreviewScene->DirectionalLight);
}

void SDungeonEditorViewport::OnShowPropertyAtmosphericFog() {
    ShowObjectProperties(AtmosphericFog);
}

void SDungeonEditorViewport::OnToggleDebugData() {
    bDrawDebugData = !bDrawDebugData;
}

void SDungeonEditorViewport::HandleForceRebuildPreviewLayout() {
    bForcePreviewRebuildLayout = !bForcePreviewRebuildLayout;
}

bool SDungeonEditorViewport::HandleForceRebuildPreviewLayoutIsChecked() const {
    return bForcePreviewRebuildLayout;
}

bool SDungeonEditorViewport::HandleToggleDebugDataIsChecked() const {
    return bDrawDebugData;
}

void SDungeonEditorViewport::ShowObjectProperties(UObject* Object, bool bForceRefresh) {
    if (DungeonEditorPtr.IsValid() && Object) {
        DungeonEditorPtr.Pin()->ShowObjectDetails(Object, bForceRefresh);
    }
}

void SDungeonEditorViewport::ListenToConfigChanges(UDungeonConfig* Config) {
    Config->ConfigPropertyChanged.Unbind();
    Config->ConfigPropertyChanged.BindUObject(ObjectToEdit->PreviewViewportProperties,
                                              &UDungeonEditorViewportProperties::PostEditChangeConfigProperty);
}

void SDungeonEditorViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
                                  const float InDeltaTime) {
    SEditorViewport::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
    if (bRequestRebuildPreviewMesh) {
        bRequestRebuildPreviewMesh = false;
        PerformMeshRebuild();
    }

    if (bDrawDebugData) {
        DungeonBuilder->DrawDebugData(GetWorld());
    }

    if (SceneProvider.IsValid()) {
        SceneProvider->RunGameThreadCommands(30);
    }
}

EVisibility SDungeonEditorViewport::GetToolbarVisibility() const {
    return EVisibility::Visible;
}

TSharedRef<FEditorViewportClient> SDungeonEditorViewport::MakeEditorViewportClient() {
    EditorViewportClient = MakeShareable(
        new FDungeonEditorViewportClient(DungeonEditorPtr, SharedThis(this), *PreviewScene, ObjectToEdit));

    EditorViewportClient->bSetListenerPosition = false;
    EditorViewportClient->SetRealtime(true); // TODO: Check if real-time is needed
    EditorViewportClient->VisibilityDelegate.BindSP(this, &SDungeonEditorViewport::IsVisible);

    return EditorViewportClient.ToSharedRef();
}

EVisibility SDungeonEditorViewport::OnGetViewportContentVisibility() const {
    return EVisibility::Visible;
}

void SDungeonEditorViewport::BindCommands() {
    SEditorViewport::BindCommands();

    const FDungeonEditorViewportCommands& ViewportActions = FDungeonEditorViewportCommands::Get();
    CommandList->MapAction(
        ViewportActions.ShowPropertyDungeon,
        FExecuteAction::CreateSP(this, &SDungeonEditorViewport::OnShowPropertyDungeon));

    CommandList->MapAction(
        ViewportActions.ShowPropertySkylight,
        FExecuteAction::CreateSP(this, &SDungeonEditorViewport::OnShowPropertySkylight));

    CommandList->MapAction(
        ViewportActions.ShowPropertyDirectionalLight,
        FExecuteAction::CreateSP(this, &SDungeonEditorViewport::OnShowPropertyDirectionalLight));

    CommandList->MapAction(
        ViewportActions.ShowPropertyAtmosphericFog,
        FExecuteAction::CreateSP(this, &SDungeonEditorViewport::OnShowPropertyAtmosphericFog));

    CommandList->MapAction(
        ViewportActions.ToggleDebugData,
        FExecuteAction::CreateSP(this, &SDungeonEditorViewport::OnToggleDebugData));

    CommandList->MapAction(
        ViewportActions.ForceRebuildPreviewLayout,
        FExecuteAction::CreateSP(this, &SDungeonEditorViewport::HandleForceRebuildPreviewLayout),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(this, &SDungeonEditorViewport::HandleForceRebuildPreviewLayoutIsChecked));

    CommandList->MapAction(
        ViewportActions.DrawDebugData,
        FExecuteAction::CreateSP(this, &SDungeonEditorViewport::OnToggleDebugData),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(this, &SDungeonEditorViewport::HandleToggleDebugDataIsChecked));
}

void SDungeonEditorViewport::OnFocusViewportToSelection() {
    SEditorViewport::OnFocusViewportToSelection();
}

TSharedPtr<SWidget> SDungeonEditorViewport::MakeViewportToolbar() {
    // Build our toolbar level toolbar
    TSharedRef<SDungeonEditorViewportToolBar> ToolBar =
        SNew(SDungeonEditorViewportToolBar)
		.Viewport(SharedThis(this))
		.Visibility(this, &SDungeonEditorViewport::GetToolbarVisibility)
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

bool SDungeonEditorViewport::IsVisible() const {
    return ViewportWidget.IsValid() && (!ParentTab.IsValid() || ParentTab.Pin()->IsForeground());
}

UDungeonModel* SDungeonEditorViewport::CreateModelInstance(UObject* Outer) {
    if (!DungeonBuilder) return nullptr;
    UClass* ModelClass = DungeonBuilder->GetModelClass();
    if (!ModelClass) return nullptr;

    return NewObject<UDungeonModel>(Outer, ModelClass);
}

UDungeonConfig* SDungeonEditorViewport::CreateConfigInstance(UObject* Outer) {
    if (!DungeonBuilder) return nullptr;
    UClass* ConfigClass = DungeonBuilder->GetConfigClass();
    if (!ConfigClass) return nullptr;

    return NewObject<UDungeonConfig>(Outer, ConfigClass);
}

UDungeonQuery* SDungeonEditorViewport::CreateQueryInstance(UObject* Outer) {
    if (!DungeonBuilder) return nullptr;
    UClass* QueryClass = DungeonBuilder->GetQueryClass();
    if (!QueryClass) return nullptr;

    return NewObject<UDungeonQuery>(Outer, QueryClass);
}

void SDungeonEditorViewport::CreateDungeonBuilder(TSubclassOf<UDungeonBuilder> BuilderClass) {
    if (BuilderClass == nullptr) {
        BuilderClass = UGridDungeonBuilder::StaticClass();
    }

    if (DungeonBuilder) {
        DungeonBuilder->RemoveFromRoot();
    }

    DungeonBuilder = NewObject<UDungeonBuilder>(static_cast<UObject*>(GetTransientPackage()), BuilderClass);
    DungeonBuilder->AddToRoot();

    if (DungeonModel) {
        DungeonModel->RemoveFromRoot();
    }
    DungeonModel = CreateModelInstance(static_cast<UObject*>(GetTransientPackage()));
    DungeonModel->AddToRoot();

    UDungeonConfig* OldConfig = ObjectToEdit->PreviewViewportProperties->DungeonConfig;
    if (!OldConfig || !OldConfig->IsValidLowLevel() || OldConfig->GetClass() != DungeonBuilder->GetConfigClass()) {
        UDungeonConfig* DungeonConfig = CreateConfigInstance(ObjectToEdit);
        ObjectToEdit->PreviewViewportProperties->DungeonConfig = DungeonConfig;

        DungeonConfig->ConfigPropertyChanged.BindUObject(ObjectToEdit->PreviewViewportProperties,
                                                         &UDungeonEditorViewportProperties::PostEditChangeConfigProperty);
    }

    if (DungeonQuery) {
        DungeonQuery->RemoveFromRoot();
    }
    DungeonQuery = CreateQueryInstance(static_cast<UObject*>(GetTransientPackage()));
    DungeonQuery->Initialize(ObjectToEdit->PreviewViewportProperties->DungeonConfig, DungeonModel,
                             FTransform::Identity);
    DungeonQuery->AddToRoot();

    DestroyDungeonActors();
}

void SDungeonEditorViewport::DestroyDungeonActors() {
    const FName DungeonTag = UDungeonModelHelper::GetDungeonIdTag(nullptr);
    // Destroy all actors that have a "Dungeon" tag
    for (TActorIterator<AActor> ActorItr(PreviewScene->GetWorld()); ActorItr; ++ActorItr) {
        if (ActorItr->ActorHasTag(DungeonTag) || ActorItr->ActorHasTag(UDungeonModelHelper::GenericDungeonIdTag)) {
            ActorItr->Destroy();
        }
    }
}

void SDungeonEditorViewport::CleanupModifiedNodeObjects() {
    const FName DungeonTag = UDungeonModelHelper::GetDungeonIdTag(nullptr);
    for (const FName& NodeId : NodeObjectsToRebuild) {
        FName NodeTag(*FString("NODE-").Append(NodeId.ToString()));
        // Destroy all actors that have a "Dungeon" tag
        for (TActorIterator<AActor> ActorItr(PreviewScene->GetWorld()); ActorItr; ++ActorItr) {
            if (ActorItr->ActorHasTag(DungeonTag) && ActorItr->ActorHasTag(NodeTag)) {
                ActorItr->Destroy();
            }
        }
    }
    NodeObjectsToRebuild.Reset();
}

void SDungeonEditorViewport::OnPropertyChanged(FString PropertyName, UDungeonEditorViewportProperties* Properties) {
    if (PropertyName == "BuilderClass") {
        CreateDungeonBuilder(Properties->BuilderClass);
        UDungeonConfig* DungeonConfig = CreateConfigInstance(ObjectToEdit);
        ObjectToEdit->PreviewViewportProperties->DungeonConfig = DungeonConfig;
        ListenToConfigChanges(DungeonConfig);
        ShowObjectProperties(ObjectToEdit->PreviewViewportProperties, true);

        // Recreate the default builder marker nodes
        if (DungeonEditorPtr.IsValid()) {
            DungeonEditorPtr.Pin()->RecreateDefaultMarkerNodes();
        }

    }
    else if (PropertyName == "Instanced") {
        DestroyDungeonActors();
    }

    DungeonModel = CreateModelInstance(ObjectToEdit);
    DungeonBuilder->BuildDungeon(DungeonModel, Properties->DungeonConfig, DungeonQuery, PreviewScene->GetWorld());
    bRequestRebuildPreviewMesh = true;
}

void SDungeonEditorViewport::SetNodesToRebuild(const TSet<FName>& NodeIds) {
    NodeObjectsToRebuild.Append(NodeIds);
}

