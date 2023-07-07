//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ComponentAssetBroker.h"

class FPlaceableMarkerAssetBroker : public IComponentAssetBroker {
public:
    virtual UClass* GetSupportedAssetClass() override;
    virtual bool AssignAssetToComponent(UActorComponent* InComponent, UObject* InAsset) override;
    virtual UObject* GetAssetFromComponent(UActorComponent* InComponent) override;
};

