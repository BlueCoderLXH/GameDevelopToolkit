//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/PlaceableMarker/PlaceableMarkerAssetTypeActions.h"

#include "DungeonArchitectEditorModule.h"
#include "Frameworks/ThemeEngine/Markers/PlaceableMarker.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FDungeonThemeAssetTypeActions

FText FPlaceableMarkerAssetTypeActions::GetName() const {
    return LOCTEXT("FPlaceableMarkerAssetTypeActionsName", "Placeable Marker");
}

FColor FPlaceableMarkerAssetTypeActions::GetTypeColor() const {
    return FColor::Emerald;
}

UClass* FPlaceableMarkerAssetTypeActions::GetSupportedClass() const {
    return UPlaceableMarkerAsset::StaticClass();
}

uint32 FPlaceableMarkerAssetTypeActions::GetCategories() {
    return IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}


#undef LOCTEXT_NAMESPACE

