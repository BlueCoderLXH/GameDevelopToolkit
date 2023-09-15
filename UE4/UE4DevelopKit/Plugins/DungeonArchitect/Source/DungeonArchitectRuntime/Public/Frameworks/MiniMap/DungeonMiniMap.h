//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "DungeonMiniMap.generated.h"

class UTextureRenderTarget2D;
class UDungeonModel;
class UDungeonConfig;
class FDungeonLayoutData;
class UTexture;
class UTexture2D;

UENUM()
enum class EDungeonMiniMapIconCoordinateSystem : uint8 {
    Pixels = 0 UMETA(DisplayName = "Pixels"),
    WorldCoordinates UMETA(DisplayName = "World Coordinates"),
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FDungeonMiniMapOverlayIcon {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    FName Name;

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    float ScreenSize = 32.0f;

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    EDungeonMiniMapIconCoordinateSystem ScreenSizeType;

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    FLinearColor Tint = FLinearColor::White;

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    float Rotation = 0.0f;
};


USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FDungeonMiniMapOverlayTracking {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
    TWeakObjectPtr<AActor> TrackedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
    FName Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
    FName IconName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
    bool bOrientToRotation = false;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API ADungeonMiniMap : public AActor {
    GENERATED_BODY()

public:
    ADungeonMiniMap();

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    int32 TextureSize = 512;

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    float OutlineThickness = 4.0f;

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    float DoorThickness = 8.0f;

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    TArray<FDungeonMiniMapOverlayIcon> OverlayIcons;

    UPROPERTY(EditAnywhere, Category = "MiniMap")
    UMaterialInterface* MaterialTemplate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MiniMap")
    TArray<FDungeonMiniMapOverlayTracking> DynamicTracking;

    UPROPERTY(EditAnywhere, Category = "MiniMap Blur Layer")
    float BlurRadius = 5;

    UPROPERTY(EditAnywhere, Category = "MiniMap Blur Layer")
    int32 BlurIterations = 3;

    UPROPERTY(EditAnywhere, Category = "Fog Of War")
    bool bEnableFogOfWar = false;

    UPROPERTY(EditAnywhere, Category = "Fog Of War", meta = (EditCondition = bEnableFogOfWar))
    float FogOfWarTextureScale = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Fog Of War", meta = (EditCondition = bEnableFogOfWar))
    FName FogOfWarTrackingItem;

    UPROPERTY(EditAnywhere, Category = "Fog Of War", meta = (EditCondition = bEnableFogOfWar))
    UTexture2D* FogOfWarExploreTexture;

    UPROPERTY(EditAnywhere, Category = "Fog Of War", meta = (EditCondition = bEnableFogOfWar))
    float FogOfWarVisiblityDistance = 3000.0f;

    UPROPERTY(Transient)
    UTexture* MaskTexture;

    UPROPERTY(Transient)
    UTextureRenderTarget2D* StaticOverlayTexture;

    UPROPERTY(Transient)
    UTextureRenderTarget2D* DynamicOverlayTexture;

    UPROPERTY(Transient)
    UTextureRenderTarget2D* FogOfWarTexture;

    UPROPERTY(BlueprintReadOnly, Category = "MiniMap")
    FTransform WorldToScreen;

public:
    UFUNCTION(BlueprintCallable, Category = Dungeon)
    UMaterialInterface* CreateMaterialInstance();

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    UMaterialInterface* CreateMaterialInstanceFromTemplate(UMaterialInterface* InMaterialTemplate);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void UpdateMaterial(UMaterialInterface* InMaterial);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    virtual void BuildLayout(UDungeonModel* DungeonModel, UDungeonConfig* DungeonConfig);

    //// Begin AActor Interface ////
    virtual void BeginDestroy() override;
    virtual void Tick(float DeltaSeconds) override;
    //// End AActor Interface ////

protected:
    void BuildLayoutTexture(const FDungeonLayoutData& LayoutData);
    void BuildStaticOverlayTexture(const FDungeonLayoutData& LayoutData);

    void UpdateDynamicOverlayTexture();
    void UpdateFogOfWarTexture();

    float GetAttributeScaleMultiplier() const;
    float GetIconPixelSize(const FDungeonMiniMapOverlayIcon& OverlayData, const FVector2D& CanvasSize);
};


UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class DUNGEONARCHITECTRUNTIME_API UDungeonMiniMapTrackedObject : public UActorComponent {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
    FName Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
    FName IconName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap")
    bool bOrientToRotation = false;

public:
    virtual void BeginPlay() override;
};

