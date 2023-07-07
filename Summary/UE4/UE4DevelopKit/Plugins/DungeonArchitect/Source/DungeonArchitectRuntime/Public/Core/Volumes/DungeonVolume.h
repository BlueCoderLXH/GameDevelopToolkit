//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Dungeon.h"

#include "GameFramework/Volume.h"
#include "DungeonVolume.generated.h"

struct FRectangle;
UCLASS(abstract, Blueprintable)
class DUNGEONARCHITECTRUNTIME_API ADungeonVolume : public AVolume {
    GENERATED_BODY()

public:
    ADungeonVolume(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    ADungeon* Dungeon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool RealtimeUpdate;

    void GetDungeonVolumeBounds(const FVector& GridCellSize, FRectangle& OutBounds) const;

    virtual void Tick(float DeltaSeconds) override;
    virtual bool ShouldTickIfViewportsOnly() const override { return true; }

#if WITH_EDITOR
    //Begin UObject Interface
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostEditMove(bool bFinished) override;
    //End UObject Interface
#endif // WITH_EDITOR

protected:
    virtual void RebuildDungeon();

};

