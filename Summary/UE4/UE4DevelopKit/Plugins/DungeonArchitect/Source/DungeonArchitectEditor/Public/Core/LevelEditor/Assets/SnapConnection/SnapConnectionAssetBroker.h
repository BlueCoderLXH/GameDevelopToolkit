//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionComponent.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionInfo.h"

#include "ComponentAssetBroker.h"

class FSnapConnectionAssetBroker : public IComponentAssetBroker {
public:

    virtual UClass* GetSupportedAssetClass() override {
        return USnapConnectionInfo::StaticClass();
    }

    virtual bool AssignAssetToComponent(UActorComponent* InComponent, UObject* InAsset) override {
        if (USnapConnectionComponent* ConnectionComponent = Cast<USnapConnectionComponent>(InComponent)) {
            if (USnapConnectionInfo* ConnectionAsset = Cast<USnapConnectionInfo>(InAsset)) {
                ConnectionComponent->ConnectionInfo = ConnectionAsset;
                return true;
            }
        }

        return false;
    }

    virtual UObject* GetAssetFromComponent(UActorComponent* InComponent) override {
        if (USnapConnectionComponent* ConnectionComponent = Cast<USnapConnectionComponent>(InComponent)) {
            return ConnectionComponent->ConnectionInfo;
        }
        return nullptr;
    }

};

