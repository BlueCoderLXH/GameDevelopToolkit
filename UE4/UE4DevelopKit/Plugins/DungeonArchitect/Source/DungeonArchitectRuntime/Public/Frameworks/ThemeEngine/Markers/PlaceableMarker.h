//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "PlaceableMarker.generated.h"

class UArrowComponent;

enum class EPlaceableMarkerAssetVersion {
    InitialVersion = 0,

    //----------- Versions should be placed above this line -----------------
    LastVersionPlusOne,
    LatestVersion = LastVersionPlusOne - 1
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FPlaceableMarkerAssetSpriteSettings {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category=Dungeon)
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, Category=Dungeon)
    float OffsetZ = 0.0f;
};


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UPlaceableMarkerAsset : public UObject {
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, Category = "Dungeon")
    TArray<FString> MarkerNames;

    UPROPERTY()
    int32 Version = 0;
    
#if WITH_EDITORONLY_DATA
    /** An editor-only sprite to attach to the placed actor. This is purely for visual purpose while designing your levels */
    UPROPERTY(EditAnywhere, Category = "Dungeon")
    UTexture2D* PreviewSprite = nullptr;

    UPROPERTY(EditAnywhere, Category = "Dungeon")
    FPlaceableMarkerAssetSpriteSettings PreviewSpriteSettings;
#endif // WITH_EDITORONLY_DATA
};

/**
 * Place a marker on to the world.   If the theme engine picks this component up (which depends on the builder),
 * then it will spawn the objects defined in the theme file under the specified marker name
 */
UCLASS(Blueprintable, HideCategories=(Rendering,Input,Actor,Misc,Replication,Collision,LOD,Cooking))
class DUNGEONARCHITECTRUNTIME_API UPlaceableMarkerComponent : public USceneComponent {
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, Category="Dungeon")
    UPlaceableMarkerAsset* MarkerAsset = nullptr;

};

UCLASS(Blueprintable, ConversionRoot, ComponentWrapperClass, HideCategories=(Rendering,Input,Actor,Misc,Replication,Collision,LOD,Cooking))
class DUNGEONARCHITECTRUNTIME_API APlaceableMarkerActor : public AActor {
    GENERATED_UCLASS_BODY()
public:
	UPROPERTY(Category=Dungeon, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Dungeon", AllowPrivateAccess = "true"))
    UPlaceableMarkerComponent* PlaceableMarkerComponent;
    
#if WITH_EDITORONLY_DATA
    UPROPERTY()
    UBillboardComponent* SpriteComponent;
    
    UPROPERTY(Transient)
    UArrowComponent* ArrowComponent;
#endif // WITH_EDITORONLY_DATA

public:
    virtual void PostLoad() override;
    virtual void PostActorCreated() override;
    virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
    TArray<FString> GetAllowedMarkerNames() const;
    
#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif

protected:
    virtual void Initialize();
};

