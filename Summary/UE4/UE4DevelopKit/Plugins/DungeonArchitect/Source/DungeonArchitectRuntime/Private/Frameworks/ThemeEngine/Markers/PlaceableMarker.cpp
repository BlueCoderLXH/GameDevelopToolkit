//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/ThemeEngine/Markers/PlaceableMarker.h"

#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"

APlaceableMarkerActor::APlaceableMarkerActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    PlaceableMarkerComponent = ObjectInitializer.CreateDefaultSubobject<UPlaceableMarkerComponent>(this, "MarkerComponent");
    PlaceableMarkerComponent->SetMobility(EComponentMobility::Static);
    RootComponent = PlaceableMarkerComponent;
    
#if WITH_EDITORONLY_DATA
    ArrowComponent = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UArrowComponent>(this, TEXT("Arrow"));
    SpriteComponent = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UBillboardComponent>( this, TEXT("Sprite"));
    
    if (!IsRunningCommandlet()) {
        if (ArrowComponent) {
            ArrowComponent->SetMobility(EComponentMobility::Static);
            ArrowComponent->SetupAttachment(RootComponent);
            ArrowComponent->SetHiddenInGame(true);
        }

        if (SpriteComponent) {
            SpriteComponent->SetMobility(EComponentMobility::Static);
            SpriteComponent->SetupAttachment(RootComponent);
            SpriteComponent->SetHiddenInGame(true);
        }
    };
#endif // WITH_EDITORONLY_DATA

}

void APlaceableMarkerActor::PostLoad() {
    Super::PostLoad();
    
    Initialize();
}

void APlaceableMarkerActor::PostActorCreated() {
    Super::PostActorCreated();
    
    Initialize();
}

void APlaceableMarkerActor::PostDuplicate(EDuplicateMode::Type DuplicateMode) {
    Super::PostDuplicate(DuplicateMode);
    
}

TArray<FString> APlaceableMarkerActor::GetAllowedMarkerNames() const {
    TArray<FString> Result;
    if (PlaceableMarkerComponent && PlaceableMarkerComponent->MarkerAsset) {
        Result = PlaceableMarkerComponent->MarkerAsset->MarkerNames;
    }
    return Result;
}

#if WITH_EDITOR
void APlaceableMarkerActor::PostEditChangeProperty(FPropertyChangedEvent& e) {
    Super::PostEditChangeProperty(e);

    Initialize();
}
#endif // WITH_EDITOR

void APlaceableMarkerActor::Initialize() {
#if WITH_EDITORONLY_DATA
    UPlaceableMarkerAsset* MarkerAsset = PlaceableMarkerComponent ? PlaceableMarkerComponent->MarkerAsset : nullptr;
    if (MarkerAsset && MarkerAsset->PreviewSprite && SpriteComponent) {
        SpriteComponent->SetSprite(MarkerAsset->PreviewSprite);
        SpriteComponent->SetRelativeScale3D(FVector(MarkerAsset->PreviewSpriteSettings.Scale));
        SpriteComponent->SetRelativeLocation(FVector(0, 0, MarkerAsset->PreviewSpriteSettings.OffsetZ));
    }
#endif // WITH_EDITORONLY_DATA
    
}

