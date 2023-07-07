//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Placements/DungeonArchitectPlacements.h"

#include "Builders/Grid/Volumes/GridDungeonPlatformVolume.h"
#include "Core/Dungeon.h"
#include "Core/Volumes/DungeonMarkerReplaceVolume.h"
#include "Core/Volumes/DungeonMirrorVolume.h"
#include "Core/Volumes/DungeonNegationVolume.h"
#include "Core/Volumes/DungeonThemeOverrideVolume.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "IPlacementModeModule.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectPlacements"

//////////////////////////////// FDungeonItemsPlacements //////////////////////////////// 

void FDAPlacements::Initialize() {
    if (IPlacementModeModule::IsAvailable()) {
        HandlePlacements();
    }
    else {
        // The placements module is not available yet.  Wait for it to load
        FModuleManager::Get().OnModulesChanged().AddRaw(this, &FDAPlacements::OnModulesChanged);
    }
}

void FDAPlacements::Release() {
    FModuleManager::Get().OnModulesChanged().RemoveAll(this);

    if (IPlacementModeModule::IsAvailable()) {
        IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();
        PlacementModeModule.UnregisterPlacementCategory(GetCategoryName());
    }
}

void FDAPlacements::GetAssetList(const FString& Path, TArray<FAssetData>& OutAssetList) {
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(
        TEXT("AssetRegistry"));
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    AssetRegistry.ScanPathsSynchronous({Path});
    AssetRegistry.GetAssetsByPath(*Path, OutAssetList);
}

void FDAPlacements::OnModulesChanged(FName Module, EModuleChangeReason Reason) {
    if (Module == TEXT("PlacementMode") && Reason == EModuleChangeReason::ModuleLoaded) {
        HandlePlacements();
    }
}

//////////////////////////////// FDungeonItemsPlacements //////////////////////////////// 
FName FDungeonItemsPlacements::GetCategoryName() const { return TEXT("PMCDungeonItems"); }

void FDungeonItemsPlacements::HandlePlacements() {
    IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();

    PlacementModeModule.RegisterPlacementCategory(
        FPlacementCategoryInfo(
            LOCTEXT("DAItemsPlacementModeLabel", "Dungeon Architect"),
            GetCategoryName(),
            "TagDACategory",
            1000
        )
    );

    int32 SortOrder = 0;

#define ADD_PLACEABLE_ITEM(Class) PlacementModeModule.RegisterPlaceableItem(GetCategoryName(), MakeShareable(new FPlaceableItem(nullptr, FAssetData(Class::StaticClass()), SortOrder += 10)))

    ADD_PLACEABLE_ITEM(ADungeon);
    ADD_PLACEABLE_ITEM(AGridDungeonPlatformVolume);
    ADD_PLACEABLE_ITEM(ADungeonThemeOverrideVolume);
    ADD_PLACEABLE_ITEM(ADungeonMarkerReplaceVolume);
    ADD_PLACEABLE_ITEM(ADungeonMirrorVolume);
    ADD_PLACEABLE_ITEM(ADungeonNegationVolume);

#undef ADD_PLACEABLE_ITEM
}


//////////////////////////////// FDAProtoToolsMeshPlacements //////////////////////////////// 


void FDAProtoToolsMeshPlacements::HandlePlacements() {
    IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();

    PlacementModeModule.RegisterPlacementCategory(
        FPlacementCategoryInfo(
            LOCTEXT("ProtoToolsMeshPlacementModeLabel", "DA Proto Meshs"),
            GetCategoryName(),
            "TagDAProtoToolsMeshCategory",
            1100
        )
    );

    TArray<FAssetData> AssetList;
    GetAssetList("/DungeonArchitect/ProtoTools/Meshes", AssetList);

    int32 SortOrder = 0;
    for (const FAssetData& Asset : AssetList) {
        PlacementModeModule.RegisterPlaceableItem(GetCategoryName(),
                                                  MakeShareable(new FPlaceableItem(nullptr, Asset, SortOrder += 10)));
    }

}


//////////////////////////////// FDAProtoToolsMaterialPlacements //////////////////////////////// 

void FDAProtoToolsMaterialPlacements::HandlePlacements() {
    IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();

    PlacementModeModule.RegisterPlacementCategory(
        FPlacementCategoryInfo(
            LOCTEXT("ProtoToolsMaterialPlacementModeLabel", "DA Proto Materials"),
            GetCategoryName(),
            "TagDAProtoToolsMaterialCategory",
            1200
        )
    );

    TArray<FAssetData> AssetList;
    GetAssetList("/DungeonArchitect/ProtoTools/Materials", AssetList);

    int32 SortOrder = 0;
    for (const FAssetData& Asset : AssetList) {
        PlacementModeModule.RegisterPlaceableItem(GetCategoryName(),
                                                  MakeShareable(new FPlaceableItem(nullptr, Asset, SortOrder += 10)));
    }

}


#undef LOCTEXT_NAMESPACE

