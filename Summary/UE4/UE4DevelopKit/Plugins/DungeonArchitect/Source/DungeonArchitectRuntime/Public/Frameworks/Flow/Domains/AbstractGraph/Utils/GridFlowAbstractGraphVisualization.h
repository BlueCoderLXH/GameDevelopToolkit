//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GridFlowAbstractGraphVisualization.generated.h"

class UTextRenderComponent;
class UGridFlowAbstractGraph3D;
class UFlowAbstractNode;
class UFlowGraphItem;

struct DUNGEONARCHITECTRUNTIME_API FGFAbstractGraphVisualizerSettings {
    float NodeRadius = 40.0f;
    float ItemScale = 0.25f;
    float LinkPadding = 10.0f;
    float LinkThickness = 8.0f;
    float LinkRefThickness = 4.0f;
    FLinearColor OneWayLinkColor = FLinearColor(0.4f, 0.05f, 0.0f);
    FVector NodeSeparationDistance = FVector(400, 400, 400);
    bool bRenderNodeOnCellCenter = false;
};

UCLASS(HideDropDown, NotPlaceable, NotBlueprintable)
class DUNGEONARCHITECTRUNTIME_API UFDAbstractNodePreview : public USceneComponent {
    GENERATED_UCLASS_BODY()

public:
    void AlignToCamera(const FVector& InCameraLocation);
    void SetNodeState(const UFlowAbstractNode* InNode);
    void SetItemState(const UFlowGraphItem* InItem) const;
    void SetNodeColor(const FLinearColor& InColor) const;
    void SetOpacity(float InOpacity) const;
    void SetBorderSize(float InSize) const;
    void SetAlignToCameraEnabled(bool bEnabled) { bAlignToCamera = bEnabled; }
    void SetSelected(bool bInSelected);

public:
    UPROPERTY()
    UStaticMeshComponent* NodeMesh;

    UPROPERTY()
    UStaticMeshComponent* BoundsMesh;
    
    UPROPERTY()
    UTextRenderComponent* TextRenderer;

    UPROPERTY()
    UMaterialInterface* TextMaterial;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* DefaultMaterial;
    
    UPROPERTY(Transient)
    UMaterialInstanceDynamic* SelectedMaterial;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* BoundsMaterial;
    
    bool bAlignToCamera = true;
    bool bSelected = false;
    bool bActiveNode = true;
    
};

UCLASS(HideDropDown, NotPlaceable, NotBlueprintable)
class DUNGEONARCHITECTRUNTIME_API UFDAbstractLink : public USceneComponent {
    GENERATED_UCLASS_BODY()
public:
    void SetState(const FVector& InStart, const FVector& InEnd, float InThickness, const FLinearColor& InColor, int32 InNumHeads = 0);
    void SetLinkVisibility(bool bInVisible);
    void SetLinkColor(const FLinearColor& InColor) const;
    void AlignToCamera(const FVector& InCameraLocation, const FGFAbstractGraphVisualizerSettings& InSettings);
    void UseHeadMaterial(int32 NumHeads) const;
    void SetDynamicAlignment(USceneComponent* Start, USceneComponent* End);
    
public:
    UPROPERTY()
    UStaticMeshComponent* LineMesh;
    
    UPROPERTY(Transient)
    UMaterialInstanceDynamic* LineMaterial;
    
    UPROPERTY(Transient)
    UMaterialInstanceDynamic* HeadMaterial;

    FVector StartLocation = FVector::ZeroVector;
    FVector EndLocation = FVector::ZeroVector;
    float Thickness = 0;
    int32 NumHeads = 0;
    FLinearColor Color = FLinearColor::Black;

    TWeakObjectPtr<UFDAbstractLink> HeadComponent;
    
    bool bDynamicUpdate;
    TWeakObjectPtr<USceneComponent> DynamicCompStart;
    TWeakObjectPtr<USceneComponent> DynamicCompEnd;
    static const float MeshSize;
};

UCLASS(HideDropDown, NotPlaceable, NotBlueprintable)
class DUNGEONARCHITECTRUNTIME_API AGridFlowAbstractGraphVisualizer : public AActor {
    GENERATED_UCLASS_BODY()
public:
    virtual bool ShouldTickIfViewportsOnly() const override { return true; }
    virtual void Tick(float DeltaSeconds) override;

    void Generate(UGridFlowAbstractGraph3D* InGraph, const FGFAbstractGraphVisualizerSettings& InSettings);
    void SetAutoAlignToLevelViewport(bool bEnabled) { bAutoAlignToLevelViewport = bEnabled; }
    void AlignToCamera(const FVector& InCameraLocation) const;

    FGFAbstractGraphVisualizerSettings GetSettings() const { return Settings; }
    FVector GetNodeLocation(const UFlowAbstractNode* InNode) const;

public:
    UPROPERTY()
    FGuid DungeonID;

    
private:
    UPROPERTY()
    USceneComponent* SceneRoot = nullptr;

    UPROPERTY()
    bool bAutoAlignToLevelViewport = false;
    
    FGFAbstractGraphVisualizerSettings Settings;
};

