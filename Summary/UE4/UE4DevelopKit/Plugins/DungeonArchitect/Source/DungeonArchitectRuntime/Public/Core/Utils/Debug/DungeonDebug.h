//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonDebug.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDungeonDebug, Log, All);

UCLASS(Experimental, NotBlueprintable, hidecategories = (Rendering, Input, Actor, Misc))
class DUNGEONARCHITECTRUNTIME_API ADungeonDebug : public AActor {
    GENERATED_BODY()

public:
    ADungeonDebug(const FObjectInitializer& ObjectInitializer);

    void ExecuteDebugCommand(int32 CommandID);

    void ExecuteCommand0();

    void ExecuteCommand1() {
    }

    void ExecuteCommand2() {
    }

    void ExecuteCommand3() {
    }

    void ExecuteCommand4() {
    }

    void ExecuteCommand5();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
    AActor* CloneTemplate;

};

