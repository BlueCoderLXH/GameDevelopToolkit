//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class DUNGEONARCHITECTEDITOR_API FSnapGridFlowAssetTypeActions : public FAssetTypeActions_Base {
    public:
    // IAssetTypeActions interface
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
    virtual uint32 GetCategories() override;
    virtual const TArray<FText>& GetSubMenus() const override;
    // End of IAssetTypeActions interface
};

class DUNGEONARCHITECTEDITOR_API FSnapGridFlowModuleBoundsAssetTypeActions : public FAssetTypeActions_Base {
    public:
    // IAssetTypeActions interface
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual uint32 GetCategories() override;
    virtual const TArray<FText>& GetSubMenus() const override;
    // End of IAssetTypeActions interface
};


class DUNGEONARCHITECTEDITOR_API FSnapGridFlowModuleDatabaseTypeActions : public FAssetTypeActions_Base {
    public:
    // IAssetTypeActions interface
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual uint32 GetCategories() override;
    virtual const TArray<FText>& GetSubMenus() const override;
    // End of IAssetTypeActions interface
};

