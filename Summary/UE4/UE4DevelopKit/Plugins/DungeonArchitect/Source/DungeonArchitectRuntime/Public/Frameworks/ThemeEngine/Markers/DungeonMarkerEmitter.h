//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonBuilder.h"
#include "Core/DungeonModel.h"
#include "Core/DungeonQuery.h"
#include "DungeonMarkerEmitter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(DungeonMarkerEmitterLog, Log, All);

/**
* Implement this class in blueprint (or C++) to emit your own custom markers in the scene
*/
UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, abstract)
class DUNGEONARCHITECTRUNTIME_API UDungeonMarkerEmitter : public UObject {
    GENERATED_BODY()

public:

    /** Called by the theming engine to emit markers */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void EmitMarkers(UDungeonBuilder* Builder, UDungeonModel* Model, UDungeonConfig* Config, UDungeonQuery* Query);
    virtual void EmitMarkers_Implementation(UDungeonBuilder* Builder, UDungeonModel* Model, UDungeonConfig* Config,
                                            UDungeonQuery* Query);
};

