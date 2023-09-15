//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class FDAPlacements {
public:
    virtual ~FDAPlacements() {
    }

    void Initialize();
    void Release();

protected:
    virtual void HandlePlacements() = 0;
    virtual FName GetCategoryName() const = 0;

protected:
    void GetAssetList(const FString& Path, TArray<struct FAssetData>& OutAssetList);

private:
    void OnModulesChanged(FName Module, EModuleChangeReason Reason);
};


class FDungeonItemsPlacements : public FDAPlacements {
private:
    virtual void HandlePlacements() override;
    virtual FName GetCategoryName() const override;

};

class FDAProtoToolsMeshPlacements : public FDAPlacements {
private:
    virtual void HandlePlacements() override;
    virtual FName GetCategoryName() const override { return TEXT("PMCProtoToolsMesh"); }

};


class FDAProtoToolsMaterialPlacements : public FDAPlacements {
private:
    virtual void HandlePlacements() override;
    virtual FName GetCategoryName() const override { return TEXT("PMCProtoToolsMaterial"); }

};

