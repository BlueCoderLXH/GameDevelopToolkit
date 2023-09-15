//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/GridFlow/GridFlowAssetFactory.h"

#include "Builders/GridFlow/GridFlowAsset.h"
#include "Core/Editors/FlowEditor/FlowEditorUtils.h"

UGridFlowAssetFactory::UGridFlowAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    SupportedClass = UGridFlowAsset::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool UGridFlowAssetFactory::CanCreateNew() const {
    return true;
}

UObject* UGridFlowAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
                                                 UObject* Context, FFeedbackContext* Warn) {
    UGridFlowAsset* NewAsset = NewObject<UGridFlowAsset>(InParent, Class, Name, Flags | RF_Transactional);
    FFlowEditorUtils::InitializeFlowAsset(NewAsset);
    return NewAsset;
}

