//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonModel.h"
#include "GridFlowModel.generated.h"

class UGridFlowAbstractGraph;
class UGridFlowTilemap;

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridFlowModel : public UDungeonModel {
    GENERATED_BODY()
public:

    virtual void Reset() override;

public:
    UPROPERTY()
    UGridFlowAbstractGraph* AbstractGraph;

    UPROPERTY()
    UGridFlowTilemap* Tilemap;

    UPROPERTY()
    FIntPoint BuildTileOffset = FIntPoint::ZeroValue;

};

