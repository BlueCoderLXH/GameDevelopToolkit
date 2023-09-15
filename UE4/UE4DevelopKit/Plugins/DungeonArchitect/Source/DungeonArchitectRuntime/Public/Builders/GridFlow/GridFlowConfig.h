//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/GridFlow/GridFlowAsset.h"
#include "Core/DungeonConfig.h"
#include "GridFlowConfig.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GridFlowConfigLog, Log, All);


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridFlowConfig : public UDungeonConfig {
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TSoftObjectPtr<UGridFlowAsset> GridFlow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MaxRetries = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FVector GridSize = FVector(400, 400, 200);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TMap<FString, FString> ParameterOverrides;

    /**
       If true, the center of the generated dungeon would be around the Dungeon actor.  
       If false, the dungeon will not be centered and you will get a more predictable position, with reference to the tilemap

       Disable this if you are trying to blend static world geometry with your dungeon
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool bAlignDungeonAtCenter = true;
};

