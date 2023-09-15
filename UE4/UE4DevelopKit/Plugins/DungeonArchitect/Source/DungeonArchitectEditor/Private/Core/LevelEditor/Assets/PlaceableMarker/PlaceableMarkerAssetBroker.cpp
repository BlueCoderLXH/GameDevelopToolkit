//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/PlaceableMarker/PlaceableMarkerAssetBroker.h"

#include "Frameworks/ThemeEngine/Markers/PlaceableMarker.h"

UClass* FPlaceableMarkerAssetBroker::GetSupportedAssetClass() {
    return UPlaceableMarkerAsset::StaticClass();
}

bool FPlaceableMarkerAssetBroker::AssignAssetToComponent(UActorComponent* InComponent, UObject* InAsset) {
    if (UPlaceableMarkerComponent* MarkerComponent = Cast<UPlaceableMarkerComponent>(InComponent)) {
        if (UPlaceableMarkerAsset* MarkerAsset = Cast<UPlaceableMarkerAsset>(InAsset)) {
            MarkerComponent->MarkerAsset = MarkerAsset;
            return true;
        }
    }

    return false;
}

UObject* FPlaceableMarkerAssetBroker::GetAssetFromComponent(UActorComponent* InComponent) {
    if (UPlaceableMarkerComponent* ConnectionComponent = Cast<UPlaceableMarkerComponent>(InComponent)) {
        return ConnectionComponent->MarkerAsset;
    }
    return nullptr;
}

