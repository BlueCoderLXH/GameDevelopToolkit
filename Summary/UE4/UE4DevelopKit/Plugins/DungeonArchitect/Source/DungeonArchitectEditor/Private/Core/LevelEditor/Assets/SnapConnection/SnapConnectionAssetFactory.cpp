//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/SnapConnection/SnapConnectionAssetFactory.h"

#include "Frameworks/Snap/Lib/Connection/SnapConnectionInfo.h"

USnapConnectionAssetFactory::USnapConnectionAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(
    ObjectInitializer) {
    SupportedClass = USnapConnectionInfo::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool USnapConnectionAssetFactory::CanCreateNew() const {
    return true;
}

UObject* USnapConnectionAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
                                                          EObjectFlags Flags, UObject* Context,
                                                          FFeedbackContext* Warn) {
    USnapConnectionInfo* NewAsset = NewObject<USnapConnectionInfo>(InParent, Class, Name, Flags | RF_Transactional);
    NewAsset->Version = static_cast<int32>(ESnapConnectionInfoVersion::LatestVersion);
    return NewAsset;
}

