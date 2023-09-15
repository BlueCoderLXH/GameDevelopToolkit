//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/SnapGridFlow/SnapGridFlowAssetTypeActions.h"

#include "Builders/SnapGridFlow/SnapGridFlowAsset.h"
#include "Core/Common/ContentBrowserMenuExtensions.h"
#include "Core/Editors/FlowEditor/BaseEditors/SnapGridFlowEditor.h"
#include "DungeonArchitectEditorModule.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleBounds.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleDatabase.h"

#define LOCTEXT_NAMESPACE "SnapGridFlowFlowAssetTypeActions"

///////////////////////////////// FSnapGridFlowFlowAssetTypeActions /////////////////////////////////
FText FSnapGridFlowAssetTypeActions::GetName() const {
    return LOCTEXT("FlowAsset_Name", "Snap Grid - Flow Graph");
}

FColor FSnapGridFlowAssetTypeActions::GetTypeColor() const {
    return FColor::Purple;
}

UClass* FSnapGridFlowAssetTypeActions::GetSupportedClass() const {
    return USnapGridFlowAsset::StaticClass();
}

void FSnapGridFlowAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) {
    FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);

    const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
                                        ? EToolkitMode::WorldCentric
                                        : EToolkitMode::Standalone;
    for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt) {
        USnapGridFlowAsset* SnapGridFlowAsset = Cast<USnapGridFlowAsset>(*ObjIt);
        if (SnapGridFlowAsset) {
            TSharedRef<FSnapGridFlowEditor> NewGridFlowEditor(new FSnapGridFlowEditor());
            NewGridFlowEditor->InitEditor(Mode, EditWithinLevelEditor, SnapGridFlowAsset);
        }
    }
}

uint32 FSnapGridFlowAssetTypeActions::GetCategories() {
    return IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}

const TArray<FText>& FSnapGridFlowAssetTypeActions::GetSubMenus() const {
    static const TArray<FText> SubMenus = {
        FDAContentBrowserSubMenuNames::SnapGridFlow
    };
    return SubMenus;
}

///////////////////////////////// FSnapGridFlowModuleBoundsAssetTypeActions /////////////////////////////////
FText FSnapGridFlowModuleBoundsAssetTypeActions::GetName() const {
    return LOCTEXT("FlowModuleBounds_Name", "Snap Grid - Module Bounds");
}

FColor FSnapGridFlowModuleBoundsAssetTypeActions::GetTypeColor() const {
    return FColor::Red;
}

UClass* FSnapGridFlowModuleBoundsAssetTypeActions::GetSupportedClass() const {
    return USnapGridFlowModuleBoundsAsset::StaticClass();
}

uint32 FSnapGridFlowModuleBoundsAssetTypeActions::GetCategories() {
    return IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}

const TArray<FText>& FSnapGridFlowModuleBoundsAssetTypeActions::GetSubMenus() const {
    static const TArray<FText> SubMenus = {
        FDAContentBrowserSubMenuNames::SnapGridFlow
    };
    return SubMenus;
}


///////////////////////////////// FSnapGridFlowModuleDatabaseTypeActions /////////////////////////////////
FText FSnapGridFlowModuleDatabaseTypeActions::GetName() const {
    return LOCTEXT("FlowModuleDatabase_Name", "Snap Grid - Module Database");
}

FColor FSnapGridFlowModuleDatabaseTypeActions::GetTypeColor() const {
    return FColor(64, 64, 255);
}

UClass* FSnapGridFlowModuleDatabaseTypeActions::GetSupportedClass() const {
    return USnapGridFlowModuleDatabase::StaticClass();
}

uint32 FSnapGridFlowModuleDatabaseTypeActions::GetCategories() {
    return IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}

const TArray<FText>& FSnapGridFlowModuleDatabaseTypeActions::GetSubMenus() const {
    static const TArray<FText> SubMenus = {
        FDAContentBrowserSubMenuNames::SnapGridFlow
    };
    return SubMenus;
}


#undef LOCTEXT_NAMESPACE

