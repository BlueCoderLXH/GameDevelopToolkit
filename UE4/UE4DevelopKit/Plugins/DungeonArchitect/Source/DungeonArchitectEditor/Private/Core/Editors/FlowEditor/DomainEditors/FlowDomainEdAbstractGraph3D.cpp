//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdAbstractGraph3D.h"

#include "Core/Editors/FlowEditor/DomainEditors/Widgets/SFlowDomainEdViewport.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph3D.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Utils/GridFlowAbstractGraphVisualization.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"

#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "FlowDomainEdAbstractGraph3D"
DEFINE_LOG_CATEGORY_STATIC(LogAbstractGraphEd3D, Log, All);


//////////////////////////////// FFlowDomainEdAbstractGraph3D ////////////////////////////////
void FFlowDomainEdAbstractGraph3D::InitializeImpl(TSharedPtr<IDetailsView> PropertyEditor) {
    Viewport = SNew(SFlowDomainEdViewport);
    Viewport->GetViewportClient()->SetViewMode(VMI_Lit);
    Viewport->GetActorSelectionChanged().BindSP(SharedThis(this), &FFlowDomainEdAbstractGraph3D::OnActorSelectionChanged);
    Viewport->GetActorDoubleClicked().BindSP(SharedThis(this), &FFlowDomainEdAbstractGraph3D::OnActorDoubleClicked);

    UWorld* World = Viewport->GetWorld();
    if (World) {
        UStaticMesh* SkyboxMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr,
            TEXT("/DungeonArchitect/Samples/DA_SnapMap_GameDemo/Art/Meshes/Skybox")));
        Skybox = World->SpawnActor<AStaticMeshActor>();
        Skybox->GetStaticMeshComponent()->SetStaticMesh(SkyboxMesh);
        Skybox->SetActorScale3D(FVector(100.0f, 100.0f, 100.0f));
        
        UMaterialInterface* SkyboxMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
            TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Materials/M_AbstractGraph3DSkybox_Inst")));
        Skybox->GetStaticMeshComponent()->SetMaterial(0, SkyboxMaterial);
    }
    
    ContentWidget = SNew(SOverlay)
    // The graph panel
    +SOverlay::Slot()
    [
        Viewport.ToSharedRef()
    ]
    +SOverlay::Slot()
    .Padding(10)
    .VAlign(VAlign_Bottom)
    .HAlign(HAlign_Right)
    [
        SNew(STextBlock)
        .Visibility( EVisibility::HitTestInvisible )
        .TextStyle( FEditorStyle::Get(), "Graph.CornerText" )
        .Text( LOCTEXT("CornerText", "Layout Graph") )
    ];
}

FFlowDomainEditorTabInfo FFlowDomainEdAbstractGraph3D::GetTabInfo() const {
    static const FFlowDomainEditorTabInfo TabInfo = {
        TEXT("AbstractGraph3DTab"),
        LOCTEXT("LayoutGraph3DTabLabel", "Layout Graph 3D"),
        FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details")
    };

    return TabInfo;
}

TSharedRef<SWidget> FFlowDomainEdAbstractGraph3D::GetContentWidget() {
    return ContentWidget.ToSharedRef();
}

void FFlowDomainEdAbstractGraph3D::Build(FFlowExecNodeStatePtr State) {
    if (!State.IsValid()) return;

    UWorld* World = Viewport->GetWorld();
    
    AGridFlowAbstractGraphVisualizer* Visualization = nullptr;
    // Find or create the visualization actor
    {
        for (TActorIterator<AGridFlowAbstractGraphVisualizer> It(World); It; ++It) {
            Visualization = *It;
            break;
        }
        if (!Visualization) {
            Visualization = World->SpawnActor<AGridFlowAbstractGraphVisualizer>();
        }
    }

    if (!Visualization) {
        return;
    }

    UGridFlowAbstractGraph3D* Graph = State->GetState<UGridFlowAbstractGraph3D>(UFlowAbstractGraphBase::StateTypeID);
    if (!Graph) {
        return;
    }
    
    Visualization->Generate(Graph, FGFAbstractGraphVisualizerSettings());
}

void FFlowDomainEdAbstractGraph3D::RecenterView(FFlowExecNodeStatePtr State) {
    if (!State.IsValid()) {
        return;
    }
    UGridFlowAbstractGraph3D* ScriptGraph = State->GetState<UGridFlowAbstractGraph3D>(UFlowAbstractGraphBase::StateTypeID);
    if (ScriptGraph) {
        FBox TotalGridBounds(EForceInit::ForceInit);
        FBox ActiveGridBounds(EForceInit::ForceInit);

        int32 NumActiveNodes = 0;
        bool bHasActiveNodes = false;
        for (const UFlowAbstractNode* Node : ScriptGraph->GraphNodes) {
            if (!Node) continue;
            TotalGridBounds += Node->PreviewLocation;
            if (Node->bActive) {
                NumActiveNodes++;
                bHasActiveNodes = true;
                ActiveGridBounds += Node->PreviewLocation;
                for (UFlowAbstractNode* const SubNode : Node->MergedCompositeNodes) {
                    if (!SubNode) continue;
                    ActiveGridBounds += SubNode->PreviewLocation;
                }
            }
        }

        FBox Focus = bHasActiveNodes ? ActiveGridBounds : TotalGridBounds;
        const float ExpandAmount = (NumActiveNodes == 1) ? 100 : 50;
        Focus = Focus.ExpandBy(FVector(ExpandAmount));
        Viewport->GetViewportClient()->FocusViewportOnBox(Focus);
    }
}

void FFlowDomainEdAbstractGraph3D::Tick(float DeltaTime) {
    UWorld* World = Viewport.IsValid() ? Viewport->GetWorld() : nullptr;
    if (World) {
        for (TActorIterator<AGridFlowAbstractGraphVisualizer> It(World); It; ++It) {
            AGridFlowAbstractGraphVisualizer* Visualizer = *It;
            const FVector ViewLocation = Viewport->GetViewportClient()->GetViewLocation();
            Visualizer->AlignToCamera(ViewLocation);
            break;
        }
    }
}

void FFlowDomainEdAbstractGraph3D::AddReferencedObjects(FReferenceCollector& Collector) {
    if (Skybox != nullptr) {
        Collector.AddReferencedObject(Skybox);
    }
}

IFlowDomainPtr FFlowDomainEdAbstractGraph3D::CreateDomain() const {
    return MakeShareable(new FGridFlowAbstractGraph3DDomain);
}

void FFlowDomainEdAbstractGraph3D::OnActorSelectionChanged(AActor* InActor) {
    // Deselect the old actor
    if (SelectedNode.IsValid()) {
        SelectedNode->SetSelected(false);
    }
    
    UFDAbstractNodePreview* NewSelection = Cast<UFDAbstractNodePreview>(InActor);
    if (NewSelection) {
        SelectedNode = NewSelection;
        SelectedNode->SetSelected(true);
    }
    else {
        SelectedNode = nullptr;
    }
}

void FFlowDomainEdAbstractGraph3D::OnActorDoubleClicked(AActor* InActor) {
    if (InActor) {
        UE_LOG(LogAbstractGraphEd3D, Log, TEXT("Actor DoubleClick: %s"), *InActor->GetName());
    }
}


#undef LOCTEXT_NAMESPACE

