//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/SnapGridFlow/SnapGridFlowAssetFactories.h"

#include "Builders/SnapGridFlow/SnapGridFlowAsset.h"
#include "Core/Editors/FlowEditor/FlowEditorUtils.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleBounds.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleDatabase.h"

////////////////////////////////// USnapGridFlowAssetFactory //////////////////////////////////
USnapGridFlowAssetFactory::USnapGridFlowAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    SupportedClass = USnapGridFlowAsset::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool USnapGridFlowAssetFactory::CanCreateNew() const {
    return true;
}

UObject* USnapGridFlowAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
                                                     UObject* Context, FFeedbackContext* Warn) {
    USnapGridFlowAsset* NewAsset = NewObject<USnapGridFlowAsset>(InParent, Class, Name, Flags | RF_Transactional);
    FFlowEditorUtils::InitializeFlowAsset(NewAsset);
    return NewAsset;
}

////////////////////////////////// USnapGridFlowModuleBoundsAssetFactory //////////////////////////////////
USnapGridFlowModuleBoundsAssetFactory::USnapGridFlowModuleBoundsAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    SupportedClass = USnapGridFlowModuleBoundsAsset::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* USnapGridFlowModuleBoundsAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
            EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) {
    USnapGridFlowModuleBoundsAsset* NewAsset = NewObject<USnapGridFlowModuleBoundsAsset>(InParent, Class, Name, Flags | RF_Transactional);
    return NewAsset;
}

bool USnapGridFlowModuleBoundsAssetFactory::CanCreateNew() const {
    return true;
}

////////////////////////////////// USnapGridFlowModuleDatabaseFactory //////////////////////////////////
USnapGridFlowModuleDatabaseFactory::USnapGridFlowModuleDatabaseFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    SupportedClass = USnapGridFlowModuleDatabase::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* USnapGridFlowModuleDatabaseFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
            EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) {
    USnapGridFlowModuleDatabase* NewAsset = NewObject<USnapGridFlowModuleDatabase>(InParent, Class, Name, Flags | RF_Transactional);
    return NewAsset;
}

bool USnapGridFlowModuleDatabaseFactory::CanCreateNew() const {
    return true;
}

