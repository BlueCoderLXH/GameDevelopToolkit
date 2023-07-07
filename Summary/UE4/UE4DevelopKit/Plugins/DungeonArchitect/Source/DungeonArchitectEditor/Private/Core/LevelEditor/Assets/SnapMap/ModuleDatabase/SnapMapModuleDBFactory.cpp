//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/SnapMap/ModuleDatabase/SnapMapModuleDBFactory.h"

#include "Frameworks/Snap/SnapMap/SnapMapModuleDatabase.h"

USnapMapModuleDBFactory::USnapMapModuleDBFactory(const FObjectInitializer& ObjectInitializer) : Super(
    ObjectInitializer) {
    SupportedClass = USnapMapModuleDatabase::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool USnapMapModuleDBFactory::CanCreateNew() const {
    return true;
}

UObject* USnapMapModuleDBFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
                                                   UObject* Context, FFeedbackContext* Warn) {
    USnapMapModuleDatabase* NewAsset = NewObject<USnapMapModuleDatabase>(
        InParent, Class, Name, Flags | RF_Transactional);
    return NewAsset;
}

