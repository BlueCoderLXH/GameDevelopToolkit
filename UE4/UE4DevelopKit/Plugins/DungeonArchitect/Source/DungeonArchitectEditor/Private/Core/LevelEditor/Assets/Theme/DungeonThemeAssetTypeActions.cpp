//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/Theme/DungeonThemeAssetTypeActions.h"

#include "Core/Editors/ThemeEditor/DungeonArchitectThemeEditor.h"
#include "DungeonArchitectEditorModule.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"

#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FDungeonThemeAssetTypeActions

FText FDungeonThemeAssetTypeActions::GetName() const {
    return LOCTEXT("FDungeonThemeAssetTypeActionsName", "Dungeon Theme");
}

FColor FDungeonThemeAssetTypeActions::GetTypeColor() const {
    return FColor::Cyan;
}

UClass* FDungeonThemeAssetTypeActions::GetSupportedClass() const {
    return UDungeonThemeAsset::StaticClass();
}

void FDungeonThemeAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects,
                                                    TSharedPtr<class IToolkitHost> EditWithinLevelEditor) {
    const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
                                        ? EToolkitMode::WorldCentric
                                        : EToolkitMode::Standalone;
    for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt) {
        UDungeonThemeAsset* PropData = Cast<UDungeonThemeAsset>(*ObjIt);
        if (PropData) {
            TSharedRef<FDungeonArchitectThemeEditor> NewDungeonEditor(new FDungeonArchitectThemeEditor());
            NewDungeonEditor->InitDungeonEditor(Mode, EditWithinLevelEditor, PropData);
        }
    }
}

uint32 FDungeonThemeAssetTypeActions::GetCategories() {
    return EAssetTypeCategories::Basic
        | IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}

void FDungeonThemeAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) {
}


#undef LOCTEXT_NAMESPACE

