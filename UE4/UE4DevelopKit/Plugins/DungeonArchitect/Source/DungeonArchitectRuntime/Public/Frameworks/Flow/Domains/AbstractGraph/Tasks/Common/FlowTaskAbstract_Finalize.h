//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractNode.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstractBase.h"
#include "FlowTaskAbstract_Finalize.generated.h"

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UFlowTaskAbstract_Finalize : public UFlowTaskAbstractBase {
    GENERATED_BODY()
public:
    /**
        Some of the links will be converted to one way links, to avoid the player walking around locked doors.
        Adjust this weight to modify the one-way door promotion criteria

        Variable Name: OneWayDoorPromotionWeight
    */
    UPROPERTY(EditAnywhere, Category = "Finalize Graph")
    float OneWayDoorPromotionWeight = 0.0f;

    UPROPERTY(EditAnywhere, Category=Advanced)
    bool bShowDebugData = false;

public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;


};

