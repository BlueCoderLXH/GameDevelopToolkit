//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "SnapConnectionDeprecatedStructs.generated.h"

class UStaticMesh;
class UMaterialInterface;
class AActor;

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FSnapConnectionVisualMeshInfo_DEPRECATED {
    GENERATED_USTRUCT_BODY()

    FSnapConnectionVisualMeshInfo_DEPRECATED() : StaticMesh(nullptr), MaterialOverride(nullptr), Offset(FTransform::Identity) {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    UStaticMesh* StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    UMaterialInterface* MaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    FTransform Offset;
};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FSnapConnectionVisualBlueprintInfo_DEPRECATED {
    GENERATED_USTRUCT_BODY()

    FSnapConnectionVisualBlueprintInfo_DEPRECATED() : Offset(FTransform::Identity) {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    TSubclassOf<AActor> BlueprintClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    FTransform Offset;
};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FSnapConnectionVisualInfo_DEPRECATED {
    GENERATED_USTRUCT_BODY()

    FSnapConnectionVisualInfo_DEPRECATED() : bStaticMesh(true) {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    bool bStaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    FSnapConnectionVisualMeshInfo_DEPRECATED MeshInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SnapMap)
    FSnapConnectionVisualBlueprintInfo_DEPRECATED BlueprintInfo;
};

