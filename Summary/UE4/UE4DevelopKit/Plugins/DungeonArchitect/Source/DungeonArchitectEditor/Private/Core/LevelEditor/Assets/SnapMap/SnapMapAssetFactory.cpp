//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/SnapMap/SnapMapAssetFactory.h"

#include "Builders/SnapMap/SnapMapAsset.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorUtils.h"

USnapMapAssetFactory::USnapMapAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    SupportedClass = USnapMapAsset::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool USnapMapAssetFactory::CanCreateNew() const {
    return true;
}

UObject* USnapMapAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
                                               UObject* Context, FFeedbackContext* Warn) {
    USnapMapAsset* NewAsset = NewObject<USnapMapAsset>(InParent, Class, Name, Flags | RF_Transactional);
    FSnapMapEditorUtils::InitializeFlowAsset(NewAsset);
    return NewAsset;
}

