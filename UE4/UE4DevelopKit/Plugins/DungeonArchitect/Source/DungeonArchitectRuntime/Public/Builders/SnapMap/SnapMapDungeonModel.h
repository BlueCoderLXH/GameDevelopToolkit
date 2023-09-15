//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonModel.h"
#include "Frameworks/Snap/Lib/Serialization/ConnectionSerialization.h"
#include "Frameworks/Snap/SnapMap/SnapMapGraphSerialization.h"
#include "SnapMapDungeonModel.generated.h"

struct FSnapConnectionInstance;
class UGrammarScriptGraph;
class USnapMapDungeonLevelLoadHandler;

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USnapMapDungeonModel : public UDungeonModel {
    GENERATED_UCLASS_BODY()

public:
    virtual void Reset() override;
    virtual void GenerateLayoutData(class FDungeonLayoutData& OutLayout) override;
    virtual bool ShouldAutoResetOnBuild() const override { return false; }
    
    bool SearchModuleInstance(const FGuid& InNodeId, FSnapMapModuleInstanceSerializedData& OutModuleData);

public:
    UPROPERTY()
    TArray<FSnapConnectionInstance> Connections;

    UPROPERTY()
    TArray<FSnapMapModuleInstanceSerializedData> ModuleInstances;

    UPROPERTY()
    UGrammarScriptGraph* MissionGraph;
};

