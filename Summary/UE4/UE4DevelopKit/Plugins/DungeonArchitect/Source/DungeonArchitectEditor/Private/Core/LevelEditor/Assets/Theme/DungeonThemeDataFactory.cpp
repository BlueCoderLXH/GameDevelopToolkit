//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/Theme/DungeonThemeDataFactory.h"

#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"

UDungeonThemeDataFactory::UDungeonThemeDataFactory(const FObjectInitializer& ObjectInitializer) : Super(
    ObjectInitializer) {
    SupportedClass = UDungeonThemeAsset::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool UDungeonThemeDataFactory::CanCreateNew() const {
    return true;
}

UObject* UDungeonThemeDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
                                                    UObject* Context, FFeedbackContext* Warn) {
    UDungeonThemeAsset* NewAsset = NewObject<UDungeonThemeAsset>(InParent, Class, Name, Flags | RF_Transactional);
    return NewAsset;
}

