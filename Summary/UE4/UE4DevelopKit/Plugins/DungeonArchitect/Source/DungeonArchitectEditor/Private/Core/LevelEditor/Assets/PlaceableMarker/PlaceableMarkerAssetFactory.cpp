//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/PlaceableMarker/PlaceableMarkerAssetFactory.h"

#include "Frameworks/ThemeEngine/Markers/PlaceableMarker.h"

UPlaceableMarkerAssetFactory::UPlaceableMarkerAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(
    ObjectInitializer) {
    SupportedClass = UPlaceableMarkerAsset::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool UPlaceableMarkerAssetFactory::CanCreateNew() const {
    return true;
}

UObject* UPlaceableMarkerAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
                                                          EObjectFlags Flags, UObject* Context,
                                                          FFeedbackContext* Warn) {
    UPlaceableMarkerAsset* NewAsset = NewObject<UPlaceableMarkerAsset>(InParent, Class, Name, Flags | RF_Transactional);
    NewAsset->Version = static_cast<int32>(EPlaceableMarkerAssetVersion::LatestVersion);
    return NewAsset;
}

