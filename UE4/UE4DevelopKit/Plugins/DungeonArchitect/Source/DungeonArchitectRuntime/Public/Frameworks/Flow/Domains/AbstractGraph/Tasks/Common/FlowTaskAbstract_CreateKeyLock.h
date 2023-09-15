//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstractBase.h"
#include "FlowTaskAbstract_CreateKeyLock.generated.h"

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UFlowTaskAbstract_CreateKeyLock : public UFlowTaskAbstractBase {
    GENERATED_BODY()
public:
    /**
        The path where the key resides

        Variable Name: KeyPath
    */
    UPROPERTY(EditAnywhere, Category = "Create Key-Lock")
    FString KeyPath = "main";

    /**
        The path where the lock resides

        Variable Name: LockPath
    */
    UPROPERTY(EditAnywhere, Category = "Create Key-Lock")
    FString LockPath = "main";

    /**
        The Key marker name.  Create this marker in the theme file and add your key asset

        Variable Name: KeyMarkerName
    */
    UPROPERTY(EditAnywhere, Category = "Create Key-Lock")
    FString KeyMarkerName = "Key";

    /**
        The Lock marker name.  Create this marker in the theme file and add your locked door asset

        Variable Name: LockMarkerName
    */
    UPROPERTY(EditAnywhere, Category = "Create Key-Lock")
    FString LockMarkerName = "Lock";

public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;

private:
    bool FindKeyLockSetup(const FFlowAbstractGraphQuery& GraphQuery, const FRandomStream& Random,
                          FGuid& OutKeyNodeId, FGuid& OutLockLinkId, FString& OutErrorMessage) const;
};

