//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/SnapMap/SnapMapAssetTypeActions.h"

#include "Builders/SnapMap/SnapMapAsset.h"
#include "Core/Common/ContentBrowserMenuExtensions.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditor.h"
#include "DungeonArchitectEditorModule.h"

#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "DungeonFlowAssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FSnapMapAssetTypeActions

FText FSnapMapAssetTypeActions::GetName() const {
    return LOCTEXT("FDungeonSnapFlowAssetTypeActionsName", "Snap Map - Flow Graph");
}

FColor FSnapMapAssetTypeActions::GetTypeColor() const {
    return FColor(169, 64, 228);
}

UClass* FSnapMapAssetTypeActions::GetSupportedClass() const {
    return USnapMapAsset::StaticClass();
}

void FSnapMapAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects,
                                                   TSharedPtr<class IToolkitHost> EditWithinLevelEditor) {
    const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
                                        ? EToolkitMode::WorldCentric
                                        : EToolkitMode::Standalone;
    for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt) {
        USnapMapAsset* DungeonFlow = Cast<USnapMapAsset>(*ObjIt);
        if (DungeonFlow) {
            TSharedRef<FSnapMapEditor> NewDungeonEditor(new FSnapMapEditor());
            NewDungeonEditor->InitFlowEditor(Mode, EditWithinLevelEditor, DungeonFlow);
        }
    }
}

uint32 FSnapMapAssetTypeActions::GetCategories() {
    return IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}

const TArray<FText>& FSnapMapAssetTypeActions::GetSubMenus() const {
    static const TArray<FText> SubMenus = {
        FDAContentBrowserSubMenuNames::SnapMap
    };
    return SubMenus;
}

void FSnapMapAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) {
}


#undef LOCTEXT_NAMESPACE

