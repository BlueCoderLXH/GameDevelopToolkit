//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionConstants.h"
#include "Frameworks/Snap/Lib/SnapLibrary.h"

#include "Engine/DataAsset.h"
#include "SnapMapModuleDatabase.generated.h"

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSnapMapModuleDatabaseConnectionInfo {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, Category = Module)
    FGuid ConnectionId;

    UPROPERTY(VisibleAnywhere, Category = Module)
    FTransform Transform;

    UPROPERTY(VisibleAnywhere, Category = Module)
    class USnapConnectionInfo* ConnectionInfo;

    UPROPERTY(VisibleAnywhere, Category = Module)
    ESnapConnectionConstraint ConnectionConstraint;
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSnapMapModuleDatabaseItem {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, Category = Module, meta = (ToolTip = "逻辑关卡, 影响游戏逻辑，但不影响美术表现, 由策划编辑制作"))
    TSoftObjectPtr<UWorld> LogicLevel;
    
    UPROPERTY(EditAnywhere, Category = Module, meta = (ToolTip = "表现关卡, 影响美术表现、动态关卡生成, 由美术编辑制作"))
    TSoftObjectPtr<UWorld> Level;

    UPROPERTY(EditAnywhere, Category = Module)
    FName Category = "Room";

    UPROPERTY(VisibleAnywhere, Category = Module)
    FBox ModuleBounds;

    UPROPERTY(VisibleAnywhere, Category = Module)
    TArray<FSnapMapModuleDatabaseConnectionInfo> Connections;
};

uint32 GetTypeHash(const FSnapMapModuleDatabaseItem& A);


UCLASS()
class DUNGEONARCHITECTRUNTIME_API USnapMapModuleDatabase : public UDataAsset {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = Module)
    TArray<FSnapMapModuleDatabaseItem> Modules;

};

