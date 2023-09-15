//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "FlowAsset.generated.h"

class UEdGraph;
class UGridFlowExecScript;

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UFlowAssetBase : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY()
    UGridFlowExecScript* ExecScript;

#if WITH_EDITORONLY_DATA
    UPROPERTY()
    UEdGraph* ExecEdGraph;
#endif // WITH_EDITORONLY_DATA

    static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
};

