//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/SnapConnection/SnapConnectionAssetTypeActions.h"

#include "Core/Editors/SnapConnectionEditor/SnapConnectionEditor.h"
#include "DungeonArchitectEditorModule.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionInfo.h"

#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FDungeonThemeAssetTypeActions

FText FSnapConnectionAssetTypeActions::GetName() const {
    return LOCTEXT("FSnapMapConnectionAssetTypeActionsName", "Snap Connection");
}

FColor FSnapConnectionAssetTypeActions::GetTypeColor() const {
    return FColor(153, 102, 51); // Brown
}

UClass* FSnapConnectionAssetTypeActions::GetSupportedClass() const {
    return USnapConnectionInfo::StaticClass();
}

void FSnapConnectionAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects,
                                                         TSharedPtr<class IToolkitHost> EditWithinLevelEditor) {
    const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
                                        ? EToolkitMode::WorldCentric
                                        : EToolkitMode::Standalone;
    for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt) {
        USnapConnectionInfo* DoorAsset = Cast<USnapConnectionInfo>(*ObjIt);
        if (DoorAsset) {
            TSharedRef<FSnapConnectionEditor> DoorEditor(new FSnapConnectionEditor);
            DoorEditor->InitSnapConnectionEditor(Mode, EditWithinLevelEditor, DoorAsset);
        }
    }
}

uint32 FSnapConnectionAssetTypeActions::GetCategories() {
    return IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}

void FSnapConnectionAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) {
}


#undef LOCTEXT_NAMESPACE

