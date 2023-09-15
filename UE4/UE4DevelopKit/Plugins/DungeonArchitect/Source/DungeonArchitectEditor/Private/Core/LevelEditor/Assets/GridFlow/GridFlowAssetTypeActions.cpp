//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/GridFlow/GridFlowAssetTypeActions.h"

#include "Builders/GridFlow/GridFlowAsset.h"
#include "Core/Common/ContentBrowserMenuExtensions.h"
#include "Core/Editors/FlowEditor/BaseEditors/GridFlowEditor.h"
#include "DungeonArchitectEditorModule.h"

#define LOCTEXT_NAMESPACE "GridFlowAssetTypeActions"

FText FGridFlowFlowAssetTypeActions::GetName() const {
    return LOCTEXT("FDungeonGridFlowAssetTypeActionsName", "Grid Flow Graph");
}

FColor FGridFlowFlowAssetTypeActions::GetTypeColor() const {
    return FColor::Turquoise;
}

UClass* FGridFlowFlowAssetTypeActions::GetSupportedClass() const {
    return UGridFlowAsset::StaticClass();
}

void FGridFlowFlowAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects,
                                                    TSharedPtr<class IToolkitHost> EditWithinLevelEditor) {
    FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);

    const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
                                        ? EToolkitMode::WorldCentric
                                        : EToolkitMode::Standalone;
    for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt) {
        UGridFlowAsset* GridFlowAsset = Cast<UGridFlowAsset>(*ObjIt);
        if (GridFlowAsset) {
            TSharedRef<FGridFlowEditor> NewGridFlowEditor(new FGridFlowEditor());
            NewGridFlowEditor->InitEditor(Mode, EditWithinLevelEditor, GridFlowAsset);
        }
    }
}

uint32 FGridFlowFlowAssetTypeActions::GetCategories() {
    return IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}

const TArray<FText>& FGridFlowFlowAssetTypeActions::GetSubMenus() const {
    static const TArray<FText> SubMenus = {
        FDAContentBrowserSubMenuNames::GridFlow
    };
    return SubMenus;
}

void FGridFlowFlowAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) {
}

#undef LOCTEXT_NAMESPACE

