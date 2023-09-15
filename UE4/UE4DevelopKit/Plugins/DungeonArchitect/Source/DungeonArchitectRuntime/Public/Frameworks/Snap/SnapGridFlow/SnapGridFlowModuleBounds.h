//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "SnapGridFlowModuleBounds.generated.h"

//////////////////////////// Snap Grid Flow Module Bounds ////////////////////////////

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USnapGridFlowModuleBoundsAsset : public UObject {
    GENERATED_BODY()
public:
    /** The world size of a module chunk (1x1x1).  A module can span multiple chunks (e.g 2x2x1) */
    UPROPERTY(EditAnywhere, Category="Dungeon")
    FVector ChunkSize = FVector(5000, 5000, 2000);

    /** The color of the bounds wireframe. Use this bounds as a reference while designing your module level files */
    UPROPERTY(EditAnywhere, Category="Helper Visuals (Editor Only)")
    FLinearColor BoundsWireColor = FLinearColor::Red;

    /** The color of the Door Info. Use this align the doors in your module level files */
    UPROPERTY(EditAnywhere, Category="Helper Visuals (Editor Only)")
    FLinearColor DoorColor = FLinearColor::Blue;

    /**
     * How high do you want the door to be from the lower bounds.
     * This will create a door visual indicator on the bounds actor, aiding your while designing your modules
     */
    UPROPERTY(EditAnywhere, Category="Helper Visuals (Editor Only)")
    float DoorOffsetZ = 100;
};

UCLASS(HideDropdown, hidecategories=(Physics, Lighting, LOD, Rendering, TextureStreaming, Transform, Activation, "Components|Activation"))
class DUNGEONARCHITECTRUNTIME_API USnapGridFlowModuleBoundsComponent : public UPrimitiveComponent {
    GENERATED_UCLASS_BODY()
public:
    /**
    * Specify the size of the combined room in logical node size.  E.g. (2, 1, 1) implies we try to fit this room
    * with two adjacent nodes in the x axis 
    **/ 
    UPROPERTY(EditAnywhere, Category="Dungeon")
    FIntVector NumChunks = FIntVector(1, 1, 1);

    /**
    * Indicates if the bounds are to be rendered in the editor.  You can safely turn this off if it gets in the way 
    **/ 
    UPROPERTY(EditAnywhere, Category="Dungeon")
    bool bRenderBounds = true;
    
    /** The module bounds asset reference. This is automatically assigned when you drag this asset from the content browser on to the scene */
    UPROPERTY(VisibleAnywhere, Category="Dungeon")
    TSoftObjectPtr<USnapGridFlowModuleBoundsAsset> ModuleBounds = nullptr;
    
public:
    //~ Begin USceneComponent Interface
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface
    
    //~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual bool IsEditorOnly() const override;
    // virtual uint8 GetStaticDepthPriorityGroup() const override { return SDPG_Foreground; }
    //~ End UPrimitiveComponent Interface.
};

UCLASS(HideDropdown, ConversionRoot, ComponentWrapperClass)
class DUNGEONARCHITECTRUNTIME_API ASnapGridFlowModuleBoundsActor : public AActor {
    GENERATED_UCLASS_BODY()
public:
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    USnapGridFlowModuleBoundsComponent* BoundsComponent;

#if WITH_EDITORONLY_DATA
    UPROPERTY()
    UBillboardComponent* Billboard;
#endif // WITH_EDITORONLY_DATA
};

