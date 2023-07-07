//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SnapMap/Utils/SnapMapModuleDBUtils.h"

#include "Frameworks/Snap/SnapMap/SnapMapModuleDatabase.h"
#include "Frameworks/Snap/SnapModuleDBBuilder.h"

void FSnapMapModuleDBUtils::BuildModuleDatabaseCache(USnapMapModuleDatabase* InDatabase) {
    if (!InDatabase) return;

    typedef TSnapModuleDatabaseBuilder<
        FSnapMapModuleDatabaseItem,
        FSnapMapModuleDatabaseConnectionInfo,
        SnapModuleDatabaseBuilder::FDefaultModulePolicy,
        SnapModuleDatabaseBuilder::TDefaultConnectionPolicy<FSnapMapModuleDatabaseConnectionInfo>
    > FSnapMapDatabaseBuilder;
    
    FSnapMapDatabaseBuilder::Build(InDatabase->Modules, InDatabase);
    
    InDatabase->Modify();
}

