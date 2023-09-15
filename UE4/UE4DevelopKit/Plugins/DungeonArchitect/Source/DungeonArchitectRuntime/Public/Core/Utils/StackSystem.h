//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

template<typename TTask, typename TState, typename TStaticState, typename TResult>
class TStackSystem {
public:
	TStackSystem(const TStaticState& InStaticState)
		: StaticState(InStaticState)
	{}
	
	FORCEINLINE bool IsRunning() const { return bRunning; }
	FORCEINLINE bool FoundResult() const { return bFoundResult; }
	FORCEINLINE TResult GetResult() const { return Result; }
	FORCEINLINE void Initialize(const TState& InState) {
		Stack.Push(InState);
		bRunning = true;
	}
	FORCEINLINE const TArray<TState>& GetStack() const { return Stack; }

	FORCEINLINE void PushFrame(const TState& InState) { Stack.Push(InState); }
	FORCEINLINE void FinalizeResult(const TResult& InResult) {
		check(bRunning);
		bRunning = false;
		bFoundResult = true;
		Result = InResult;
	}

	FORCEINLINE void Halt() {
		bRunning = false;
	}

	void ExecuteStep() {
		if (Stack.Num() == 0) {
			bRunning = false;
		}
		
		if (!bRunning) {
			return;
		}

		TState Top = Stack.Pop();
		TTask::Execute(Top, StaticState, *this);
	}

protected:
	TStaticState StaticState;

private:
	TArray<TState> Stack;
	bool bRunning = false;
	bool bFoundResult = false;
	TResult Result;
};


