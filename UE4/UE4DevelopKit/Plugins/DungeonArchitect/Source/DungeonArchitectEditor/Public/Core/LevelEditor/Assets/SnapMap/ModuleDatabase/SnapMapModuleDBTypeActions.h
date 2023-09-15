//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class DUNGEONARCHITECTEDITOR_API FSnapMapModuleDBTypeActions : public FAssetTypeActions_Base {
public:
    // IAssetTypeActions interface
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }
    virtual uint32 GetCategories() override;
    virtual const TArray<FText>& GetSubMenus() const override;
    // End of IAssetTypeActions interface

};

