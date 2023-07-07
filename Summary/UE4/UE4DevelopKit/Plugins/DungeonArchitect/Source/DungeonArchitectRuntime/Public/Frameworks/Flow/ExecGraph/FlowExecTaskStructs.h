//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Math/RandomStream.h"

enum class EFlowTaskExecutionResult : uint8 {
    Success,
    FailRetry,
    FailHalt
};

enum class EFlowTaskExecutionStage : uint8 {
    NotExecuted,
    WaitingToExecute,
    Executed
};

enum class EFlowTaskExecutionFailureReason : uint8 {
    Unknown,
    Timeout
};

class IFlowDomain;
typedef TSharedPtr<class FFlowExecNodeState> FFlowExecNodeStatePtr;

struct DUNGEONARCHITECTRUNTIME_API FFlowExecutionOutput {
    EFlowTaskExecutionResult ExecutionResult = EFlowTaskExecutionResult::FailHalt;
    EFlowTaskExecutionFailureReason FailureReason = EFlowTaskExecutionFailureReason::Unknown;
    FFlowExecNodeStatePtr State;
    FString ErrorMessage;
};

struct DUNGEONARCHITECTRUNTIME_API FFlowExecutionInput {
    TArray<FFlowExecutionOutput> IncomingNodeOutputs;
    const FRandomStream* Random = nullptr;
    TWeakPtr<const IFlowDomain> Domain;
};

