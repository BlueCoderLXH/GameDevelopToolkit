//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/DomainEditors/Widgets/SFlowDomainEdViewport.h"

#include "EngineUtils.h"
#include "PreviewScene.h"
#include "Widgets/Docking/SDockTab.h"

DEFINE_LOG_CATEGORY_STATIC(LogDomainEdViewport, Log, All);
#define LOCTEXT_NAMESPACE "FlowDomainEdViewportBase"

//////////////////////////// SFlowDomainEdViewportBase ////////////////////////////
void SFlowDomainEdViewport::Construct(const FArguments& InArgs) {
    PreviewScene = MakeShareable(new FPreviewScene);
    SEditorViewport::Construct(SEditorViewport::FArguments());

    BindCommands();
}

SFlowDomainEdViewport::~SFlowDomainEdViewport() {
    if (EditorViewportClient.IsValid()) {
        EditorViewportClient->Viewport = nullptr;
    }
}

void SFlowDomainEdViewport::AddReferencedObjects(FReferenceCollector& Collector) {
}

UWorld* SFlowDomainEdViewport::GetWorld() const {
    return PreviewScene->GetWorld();
}

FFDViewportActorMouseEvent& SFlowDomainEdViewport::GetActorSelectionChanged() {
    return EditorViewportClient->GetActorSelectionChanged();
}

FFDViewportActorMouseEvent& SFlowDomainEdViewport::GetActorDoubleClicked() {
    return EditorViewportClient->GetActorDoubleClicked();
}

TSharedRef<FEditorViewportClient> SFlowDomainEdViewport::MakeEditorViewportClient() {
    EditorViewportClient = MakeShareable( new SFlowDomainEdViewportClient(*PreviewScene, SharedThis(this)));
    EditorViewportClient->SetRealtime(true);
    EditorViewportClient->VisibilityDelegate.BindSP(this, &SFlowDomainEdViewport::IsVisible);
    return EditorViewportClient.ToSharedRef();
}

bool SFlowDomainEdViewport::IsVisible() const {
    return ViewportWidget.IsValid() && (!ParentTab.IsValid() || ParentTab.Pin()->IsForeground());
}


//////////////////////////// SFlowDomainEdViewportClient ////////////////////////////


SFlowDomainEdViewportClient::SFlowDomainEdViewportClient(FPreviewScene& InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewport)
    : FEditorViewportClient(nullptr, &InPreviewScene, InEditorViewport)
{
    static const float ViewportGridSize = 2048.0f;
    static const int32 ViewportCellSize = 16;
    
    // Setup defaults for the common draw helper.
    DrawHelper.bDrawPivot = false;
    DrawHelper.bDrawWorldBox = false;
    DrawHelper.bDrawKillZ = false;
    DrawHelper.bDrawGrid = true;
    DrawHelper.GridColorAxis = FColor(160, 160, 160);
    DrawHelper.GridColorMajor = FColor(144, 144, 144);
    DrawHelper.GridColorMinor = FColor(128, 128, 128);
    DrawHelper.PerspectiveGridSize = ViewportGridSize;
    DrawHelper.NumCells = DrawHelper.PerspectiveGridSize / (ViewportCellSize * 2);

    FEditorViewportClient::SetViewMode(VMI_Lit);

    EngineShowFlags.SetSnap(false);
    EngineShowFlags.CompositeEditorPrimitives = true;
    OverrideNearClipPlane(1.0f);
    bUsingOrbitCamera = true;


    const FVector CamLocation(200, 200, 200);

    // Set the initial camera position
    const FRotator OrbitRotation(-45, 100, 0);
    SetCameraSetup(
        FVector::ZeroVector,
        OrbitRotation,
        FVector(0.0f, 100, 0.0f),
        FVector::ZeroVector,
        CamLocation,
        FRotator(0, 0, 0)
    );
}

void SFlowDomainEdViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);

    // Tick the preview scene world.
    if (!GIntraFrameDebuggingGameThread) {
        PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
    }
}
void SFlowDomainEdViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) {
    FEditorViewportClient::ProcessClick(View, HitProxy, Key, Event, HitX, HitY);

    AActor* SelectedActor = nullptr;
    if(HitProxy && HitProxy->IsA(HActor::StaticGetType()))
    {
        HActor* ActorHit = static_cast<HActor*>(HitProxy);
        SelectedActor = ActorHit ? ActorHit->Actor : nullptr;
    }
    ActorSelectionChanged.ExecuteIfBound(SelectedActor);

    if (Event == IE_DoubleClick && SelectedActor) {
        ActorDoubleClicked.ExecuteIfBound(SelectedActor);
    }
}

void SFlowDomainEdViewportClient::SetupViewForRendering(FSceneViewFamily& ViewFamily, FSceneView& View) {
    FEditorViewportClient::SetupViewForRendering(ViewFamily, View);

    View.AntiAliasingMethod = AAM_FXAA; 
}

#undef LOCTEXT_NAMESPACE

