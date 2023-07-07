//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Utils/GridFlowAbstractGraphVisualization.h"

#include "Core/Utils/EditorService/IDungeonEditorService.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractNode.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph3D.h"

#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "FlowDomainAbstractGraphVis"
DEFINE_LOG_CATEGORY_STATIC(LogFDAbstractGraphVis, Log, All);

//////////////////////////////// FFDAbstractNodePreview ////////////////////////////////

UFDAbstractNodePreview::UFDAbstractNodePreview(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    struct FConstructorStatics {
        ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> DefaultMaterial;
        ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> SelectedMaterial;
        ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> TextMaterial;
        ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> BoundsMaterial;
        ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
        ConstructorHelpers::FObjectFinderOptional<UStaticMesh> BoundsMesh;
        FConstructorStatics()
            : DefaultMaterial(TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Materials/M_AbstractNode"))
            , SelectedMaterial(TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Materials/M_AbstractNode_Selected"))
            , TextMaterial(TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Materials/M_AbstractGraph_Text"))
            , BoundsMaterial(TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Materials/M_AbstractGraphNodebounds_Inst"))
            , PlaneMesh(TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Meshes/NodePlane"))
            , BoundsMesh(TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Meshes/NodeBounds"))
        {}
    };
    static FConstructorStatics ConstructorStatics;

    UMaterialInterface* MatDefaultTemplate = ConstructorStatics.DefaultMaterial.Get();
    UMaterialInterface* MatSelectedTemplate = ConstructorStatics.SelectedMaterial.Get(); 
    UMaterialInterface* MatBoundsTemplate = ConstructorStatics.BoundsMaterial.Get(); 
    
    DefaultMaterial = UMaterialInstanceDynamic::Create(MatDefaultTemplate, nullptr);
    DefaultMaterial->SetFlags(RF_Transient);
    
    SelectedMaterial = UMaterialInstanceDynamic::Create(MatSelectedTemplate, nullptr);
    SelectedMaterial->SetFlags(RF_Transient);

    BoundsMaterial = UMaterialInstanceDynamic::Create(MatBoundsTemplate, nullptr);
    BoundsMaterial->SetFlags(RF_Transient);

    TextMaterial = ConstructorStatics.TextMaterial.Get();
    
    UStaticMesh* PlaneMesh = ConstructorStatics.PlaneMesh.Get();
    NodeMesh = CreateDefaultSubobject<UStaticMeshComponent>("BackgroundPlane");
    NodeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    NodeMesh->SetStaticMesh(PlaneMesh);
    NodeMesh->SetMaterial(0, DefaultMaterial);
    NodeMesh->SetupAttachment(this);

    TextRenderer = CreateDefaultSubobject<UTextRenderComponent>("Text");
    TextRenderer->SetupAttachment(this);
    TextRenderer->HorizontalAlignment = EHTA_Center;
    TextRenderer->VerticalAlignment = EVRTA_TextCenter;
    TextRenderer->SetTextRenderColor(FColor(20, 20, 20));
    TextRenderer->SetWorldSize(50.0f);
    TextRenderer->SetRelativeLocation(FVector(1, 0, 0));
    TextRenderer->SetTextMaterial(TextMaterial);
    TextRenderer->SetText(FText::FromString(""));

    // Create the bounds mesh to render a merged composite node
    
    UStaticMesh* BoundsStaticMesh = ConstructorStatics.BoundsMesh.Get();
    BoundsMesh = CreateDefaultSubobject<UStaticMeshComponent>("BoundsMesh");
    BoundsMesh->SetStaticMesh(BoundsStaticMesh);
    BoundsMesh->SetMaterial(0, BoundsMaterial);
    BoundsMesh->SetVisibility(false);
    BoundsMesh->SetupAttachment(this);
}

void UFDAbstractNodePreview::AlignToCamera(const FVector& InCameraLocation) {
    if (bAlignToCamera) {
        FVector ActorToCam = InCameraLocation - GetComponentLocation();
        ActorToCam.Normalize();
        SetWorldRotation(ActorToCam.Rotation());
        BoundsMesh->SetWorldRotation(FRotator::ZeroRotator);
    }
}

void UFDAbstractNodePreview::SetNodeState(const UFlowAbstractNode* InNode) {
    if (InNode->bActive) {
        SetNodeColor(InNode->Color);
        SetOpacity(1.0f);
        NodeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        //TextRenderer->SetText(GetRoomTypeText(InNode->RoomType));

        if (InNode->MergedCompositeNodes.Num() > 1) {
            BoundsMesh->SetVisibility(true);
            AGridFlowAbstractGraphVisualizer* Visualizer = Cast<AGridFlowAbstractGraphVisualizer>(GetOwner());
            if (Visualizer) {
                FBox DesiredBounds(ForceInit);
                for (UFlowAbstractNode* SubNode : InNode->MergedCompositeNodes) {
                    const FVector SubNodeLocation = Visualizer->GetNodeLocation(SubNode);
                    DesiredBounds += SubNodeLocation;
                }
                FGFAbstractGraphVisualizerSettings Settings = Visualizer->GetSettings();
                FVector BoundsSize = DesiredBounds.GetExtent() * 2.0f;
                BoundsSize += FVector(Settings.NodeRadius + Settings.LinkPadding) * 2.0f;
                const FVector DesiredScale = BoundsSize / 100.0f;    // The mesh size is 100 units
                BoundsMesh->SetWorldScale3D(DesiredScale);
                BoundsMaterial->SetVectorParameterValue("Color", InNode->Color);
            }
        }
        
    }
    else {
        SetOpacity(0.3f);
        NodeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        bActiveNode = false;
    }
}

void UFDAbstractNodePreview::SetItemState(const UFlowGraphItem* InItem) const {
    SetOpacity(1.0f);
    SetNodeColor(FGridFlowItemVisuals::GetBackgroundColor(InItem));
    
    TextRenderer->SetTextRenderColor(FGridFlowItemVisuals::GetTextColor(InItem).ToFColor(false));
    TextRenderer->SetText(FText::FromString(FGridFlowItemVisuals::GetText(InItem)));
    NodeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetBorderSize(0.195f);
}

auto UFDAbstractNodePreview::SetNodeColor(const FLinearColor& InColor) const -> void {
    DefaultMaterial->SetVectorParameterValue("NodeColor", InColor);
    SelectedMaterial->SetVectorParameterValue("NodeColor", InColor);
}

void UFDAbstractNodePreview::SetOpacity(float InOpacity) const {
    DefaultMaterial->SetScalarParameterValue("MasterOpacity", InOpacity);
    SelectedMaterial->SetScalarParameterValue("MasterOpacity", InOpacity);
}

void UFDAbstractNodePreview::SetBorderSize(float InSize) const {
    DefaultMaterial->SetScalarParameterValue("BorderSize", InSize);
    SelectedMaterial->SetScalarParameterValue("BorderSize", InSize);
}

void UFDAbstractNodePreview::SetSelected(bool bInSelected) {
    bSelected = bInSelected;
    NodeMesh->SetMaterial(0, bSelected ? SelectedMaterial : DefaultMaterial);
}

/////////////////////////////////////// UFDAbstractLink ///////////////////////////////////////

const float UFDAbstractLink::MeshSize = 100;

UFDAbstractLink::UFDAbstractLink(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    struct FConstructorStatics {
        ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> LineMaterial;
        ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> HeadMaterial;
        ConstructorHelpers::FObjectFinderOptional<UStaticMesh> LinkMesh;
        FConstructorStatics()
            : LineMaterial(TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Materials/M_AbstractGraphLink_Inst"))
            , HeadMaterial(TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Materials/M_AbstractLink_ArrowHead_Inst"))
            , LinkMesh(TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Meshes/NodeLink"))
        {}
    };
    static FConstructorStatics ConstructorStatics;
    
    LineMaterial = UMaterialInstanceDynamic::Create(ConstructorStatics.LineMaterial.Get(), nullptr);
    LineMaterial->SetFlags(RF_Transient);
    
    HeadMaterial = UMaterialInstanceDynamic::Create(ConstructorStatics.HeadMaterial.Get(), nullptr);
    HeadMaterial->SetFlags(RF_Transient);
    
    UStaticMesh* LinkStaticMesh = ConstructorStatics.LinkMesh.Get();
    LineMesh = CreateDefaultSubobject<UStaticMeshComponent>("LineMesh");
    LineMesh->SetStaticMesh(LinkStaticMesh);
    LineMesh->SetMaterial(0, LineMaterial);
    LineMesh->SetupAttachment(this);
};

void UFDAbstractLink::SetState(const FVector& InStart, const FVector& InEnd, float InThickness, const FLinearColor& InColor, int32 InNumHeads) {
    StartLocation = InStart;
    EndLocation = InEnd;
    Thickness = InThickness;
    NumHeads = InNumHeads;
    Color = InColor;
    
    // Set the location
    SetRelativeLocation(InStart);
    

    // Set the scale
    const FVector StartToEnd = EndLocation - StartLocation;
    float Distance = StartToEnd.Size();
    const FVector Direction = StartToEnd / Distance;
    const FRotator Rotation = Direction.Rotation();

    if (InNumHeads > 0) {
        const float HeadSizeMultiplier = 4;
        const float HeadSize = Thickness * HeadSizeMultiplier;
        Distance -= HeadSize * InNumHeads;
    
        const FVector HeadStart = StartLocation + Direction * Distance;
        const FVector HeadEnd = EndLocation;
        const float HeadThickness = HeadSize;

        // Draw the head
        if (!HeadComponent.IsValid()) {
            HeadComponent = NewObject<UFDAbstractLink>(GetOwner());
            HeadComponent->SetupAttachment(GetOwner()->GetRootComponent());
            HeadComponent->RegisterComponent();
        }
        HeadComponent->SetState(HeadStart, HeadEnd, HeadThickness, Color);
        HeadComponent->SetWorldRotation(Rotation);
        HeadComponent->SetLinkColor(InColor);
        HeadComponent->UseHeadMaterial(InNumHeads);
    }

    // Setup the actor transform
    {
        FVector Scale = GetComponentScale();
        Scale.X = Distance / MeshSize;
        Scale.Y = Thickness / MeshSize;
        SetWorldScale3D(Scale);
        SetWorldRotation(Rotation);
    }
    SetLinkColor(InColor);
}

void UFDAbstractLink::SetLinkVisibility(bool bInVisible) {
    SetVisibility(bInVisible, true);
    LineMesh->SetVisibility(bInVisible, true);
}

void UFDAbstractLink::UseHeadMaterial(int32 InNumHeads) const {
    LineMesh->SetMaterial(0, HeadMaterial);
    HeadMaterial->SetScalarParameterValue("NumHeads", InNumHeads);
}

void UFDAbstractLink::SetDynamicAlignment(USceneComponent* Start, USceneComponent* End) {
    bDynamicUpdate = true;
    DynamicCompStart = Start;
    DynamicCompEnd = End;
}

void UFDAbstractLink::SetLinkColor(const FLinearColor& InColor) const {
    LineMaterial->SetVectorParameterValue("Color", InColor);
    HeadMaterial->SetVectorParameterValue("Color", InColor);
}

void UFDAbstractLink::AlignToCamera(const FVector& InCameraLocation, const FGFAbstractGraphVisualizerSettings& InSettings) {
    if (bDynamicUpdate && DynamicCompStart.IsValid() && DynamicCompEnd.IsValid()) {
        const FVector Start = DynamicCompStart->GetComponentLocation();
        const FVector End = DynamicCompEnd->GetComponentLocation();
        FVector Direction = End - Start;
        Direction.Normalize();

        const float ItemRadius = InSettings.NodeRadius * InSettings.ItemScale; 
        const FVector LineStart = Start + Direction * ItemRadius;
        const FVector LineEnd = End - Direction * ItemRadius;
        SetState(LineStart, LineEnd, Thickness, Color, NumHeads);
    }
    
    FVector AxisX = EndLocation - StartLocation;
    AxisX.Normalize();

    FVector AxisZ = InCameraLocation - StartLocation;
    AxisZ.Normalize();

    const FVector AxisY = FVector::CrossProduct(AxisZ, AxisX);
    AxisZ = FVector::CrossProduct(AxisX, AxisY);

    const FMatrix TransformMat(AxisX, AxisY, AxisZ, FVector::ZeroVector);
    SetWorldRotation(TransformMat.Rotator());

    if (HeadComponent.IsValid()) {
        HeadComponent->AlignToCamera(InCameraLocation, InSettings);
    }
}

///////////////////////////////////// AGridFlowAbstractGraphVisualizer /////////////////////////////////////

AGridFlowAbstractGraphVisualizer::AGridFlowAbstractGraphVisualizer(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>("Root");
    RootComponent = SceneRoot;
}


void AGridFlowAbstractGraphVisualizer::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    if (bAutoAlignToLevelViewport) {
        TSharedPtr<IDungeonEditorService> EditorService = IDungeonEditorService::Get();
        if (EditorService.IsValid()) {
            FVector CameraLocation;
            FRotator CameraRotation;
            if (EditorService->GetLevelViewportCameraInfo(GetWorld(), CameraLocation, CameraRotation)) {
                AlignToCamera(CameraLocation);
            }
        }
    }
}

void AGridFlowAbstractGraphVisualizer::AlignToCamera(const FVector& InCameraLocation) const {
    for (UActorComponent* Component : GetComponents()) {
        if (UFDAbstractNodePreview* NodePreview = Cast<UFDAbstractNodePreview>(Component)) {
            NodePreview->AlignToCamera(InCameraLocation);
        }
        if (UFDAbstractLink* LinkPreview = Cast<UFDAbstractLink>(Component)) {
            LinkPreview->AlignToCamera(InCameraLocation, Settings);
        }
    }
}

FVector AGridFlowAbstractGraphVisualizer::GetNodeLocation(const UFlowAbstractNode* InNode) const {
    FVector Coord = InNode->Coord;
    if (Settings.bRenderNodeOnCellCenter) {
        Coord += FVector(0.5f); 
    }
    return Coord * Settings.NodeSeparationDistance;
}

void AGridFlowAbstractGraphVisualizer::Generate(UGridFlowAbstractGraph3D* InGraph, const FGFAbstractGraphVisualizerSettings& InSettings) {
    Settings = InSettings;
    
    // Clear out the existing scene
    {
        TArray<UActorComponent*> ActorComponents;
        GetComponents(ActorComponents);
        for (UActorComponent* Component : ActorComponents) {
            if (Component == SceneRoot) continue;
            Component->DestroyComponent();
        }
    }
    
    if (!InGraph) {
        return;
    }

    float NodeBaseScale = (Settings.NodeRadius + Settings.LinkPadding) / 50.0f;
    TMap<FGuid, UFDAbstractNodePreview*> ItemPreviewMap;
    // Build the graph nodes
    for (const UFlowAbstractNode* Node : InGraph->GraphNodes) {
        UFDAbstractNodePreview* NodePreview = NewObject<UFDAbstractNodePreview>(this);
        NodePreview->SetRelativeLocation(GetNodeLocation(Node));
        {
            float NodeScale = NodeBaseScale;
            if (!Node->bActive) {
                NodeScale *= 0.25f;
            }
            NodePreview->SetRelativeScale3D(FVector(NodeScale));
        }
        NodePreview->SetNodeState(Node);
        NodePreview->SetupAttachment(RootComponent);

        // Create the items
        {
            const float ItemRadius = Settings.NodeRadius * Settings.ItemScale;
            int32 NumItems = Node->NodeItems.Num();
            for (int i = 0; i < NumItems; i++) {
                float Angle = (2 * PI * i) / NumItems + PI * 0.25f;
                float Offset = (Settings.NodeRadius - ItemRadius) / NodeBaseScale;
                float RY = 0, RZ = 0;
                FMath::SinCos(&RZ, &RY, Angle);
                FVector Location = FVector(5, RY * Offset, RZ * Offset);
                const UFlowGraphItem* Item = Node->NodeItems[i];
                UFDAbstractNodePreview* ItemPreview = NewObject<UFDAbstractNodePreview>(this);
                ItemPreview->SetItemState(Item);
                ItemPreview->SetRelativeLocation(Location);
                ItemPreview->SetRelativeScale3D(FVector(Settings.ItemScale));
                ItemPreview->SetAlignToCameraEnabled(false);
                ItemPreview->SetupAttachment(NodePreview);

                UFDAbstractNodePreview*& ItemActorRef = ItemPreviewMap.FindOrAdd(Item->ItemId);
                ItemActorRef = ItemPreview;
            }
        }

        if (Node->MergedCompositeNodes.Num() > 1) {
            for (const UFlowAbstractNode* CompositeNode : Node->MergedCompositeNodes) {
                UFDAbstractNodePreview* CompositeNodePreview = NewObject<UFDAbstractNodePreview>(this);
                CompositeNodePreview->SetRelativeLocation(GetNodeLocation(CompositeNode));
                CompositeNodePreview->SetRelativeScale3D(FVector(NodeBaseScale * 0.25f));
                CompositeNodePreview->SetupAttachment(RootComponent);
                
                //CompositeNodePreview->SetNodeColor(BaseCompositeNode->Color);
                CompositeNodePreview->SetNodeColor(FLinearColor::Black);
                CompositeNodePreview->SetOpacity(0.75f);
                CompositeNodePreview->NodeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                CompositeNodePreview->TextRenderer->SetText(FText::FromString(""));
            }
        }
    }
    
    // Build the graph links
    FFlowAbstractGraphQuery GraphQuery(InGraph);
    for (const UFlowAbstractLink* Link : InGraph->GraphLinks) {
        if (!Link) continue;
        
        UFlowAbstractNode* Source = GraphQuery.GetNode(Link->Source);
        UFlowAbstractNode* Dest = GraphQuery.GetNode(Link->Destination);

        if (Link->Type != EFlowAbstractLinkType::Unconnected) {
            if (Link->SourceSubNode.IsValid()) {
                for (UFlowAbstractNode* SubNode : Source->MergedCompositeNodes) {
                    if (SubNode->NodeId == Link->SourceSubNode) {
                        Source = SubNode;
                        break;
                    }
                }
            }
            if (Link->DestinationSubNode.IsValid()) {
                for (UFlowAbstractNode* SubNode : Dest->MergedCompositeNodes) {
                    if (SubNode->NodeId == Link->DestinationSubNode) {
                        Dest = SubNode;
                        break;
                    }
                }
            }
        }
        
        if (Source && Dest) {
            UFDAbstractLink* PreviewLink = NewObject<UFDAbstractLink>(this);
            FVector NodeStart = GetNodeLocation(Source);
            FVector NodeEnd = GetNodeLocation(Dest);
            PreviewLink->SetupAttachment(RootComponent);
            
            const float NodeRadius = Settings.NodeRadius;
            FVector Direction = NodeEnd - NodeStart;
            const float Distance = Direction.Size();
            Direction /= Distance;

            const FVector Start = NodeStart + (NodeRadius + Settings.LinkPadding) * Direction;
            const FVector End = NodeStart + (Distance - NodeRadius - Settings.LinkPadding) * Direction;

            if (Link->Type != EFlowAbstractLinkType::Unconnected) {
                int32 NumHeads = (Link->Type == EFlowAbstractLinkType::OneWay) ? 2 : 1;
                FLinearColor Color = FLinearColor::Black;
                if (Link->Type == EFlowAbstractLinkType::OneWay) {
                    Color = Settings.OneWayLinkColor;
                }
                PreviewLink->SetState(Start, End, Settings.LinkThickness, Color, NumHeads);
                
                // Create the items
                {
                    const float ItemRadius = Settings.NodeRadius * Settings.ItemScale;
                    int32 NumItems = Link->LinkItems.Num();
                    for (int i = 0; i < NumItems; i++) {
                        FVector Offset = FVector::ZeroVector;
                        {
                            float Angle = (2 * PI * i) / NumItems + PI * 0.5f;
                            const float ItemDistance = Settings.NodeRadius - ItemRadius;
                            FMath::SinCos(&Offset.Z, &Offset.Y, Angle);
                            Offset *= ItemDistance;
                            FRotator LineRotation = (End - Start).Rotation();
                            Offset = LineRotation.RotateVector(Offset);
                        }
                        const FVector BaseLocation = Start + (End - Start) * 0.5f;
                        const FVector Location = BaseLocation + Offset;
                        const UFlowGraphItem* Item = Link->LinkItems[i];
                        UFDAbstractNodePreview* LinkItemPreview = NewObject<UFDAbstractNodePreview>(this);
                        LinkItemPreview->SetItemState(Item);
                        LinkItemPreview->SetRelativeLocation(Location);
                        LinkItemPreview->SetWorldScale3D(FVector(Settings.ItemScale));
                        LinkItemPreview->SetupAttachment(RootComponent);

                        UFDAbstractNodePreview*& ItemActorRef = ItemPreviewMap.FindOrAdd(Item->ItemId);
                        ItemActorRef = LinkItemPreview;
                     }
                }
            }
            else {
                PreviewLink->SetLinkVisibility(false);
            }
        }
    }

    // Link up the dependent items
    TArray<UFlowGraphItem*> Items;
    InGraph->GetAllItems(Items);
    for (const UFlowGraphItem* Item : Items) {
        if (!Item) continue;
        UFDAbstractNodePreview** SearchResult = ItemPreviewMap.Find(Item->ItemId);
        if (!SearchResult) continue;
        UFDAbstractNodePreview* ItemPreview = *SearchResult;
        for (const FGuid& ReferencedItemId : Item->ReferencedItemIds) {
            UFDAbstractNodePreview** RefSearchResult = ItemPreviewMap.Find(ReferencedItemId);
            if (!RefSearchResult) continue;
            UFDAbstractNodePreview* RefItemPreview = *RefSearchResult;
            if (ItemPreview && RefItemPreview) {
                // Draw a link between the two items
                FVector Start = ItemPreview->GetComponentLocation();
                FVector End = RefItemPreview->GetComponentLocation();
                FVector Direction = End - Start;
                Direction.Normalize();

                float ItemRadius = Settings.NodeRadius * Settings.ItemScale; 
                FVector LineStart = Start + Direction * ItemRadius;
                FVector LineEnd = End - Direction * ItemRadius;
                UFDAbstractLink* RefLink = NewObject<UFDAbstractLink>(this);
                if (!LineStart.Equals(LineEnd)) {
                    RefLink->SetState(LineStart, LineEnd, Settings.LinkRefThickness, FLinearColor::Red, 1);
                    RefLink->SetDynamicAlignment(ItemPreview, RefItemPreview);
                    RefLink->SetupAttachment(RootComponent);
                }
            }
        }
    }

    ReregisterAllComponents();
}


#undef LOCTEXT_NAMESPACE

