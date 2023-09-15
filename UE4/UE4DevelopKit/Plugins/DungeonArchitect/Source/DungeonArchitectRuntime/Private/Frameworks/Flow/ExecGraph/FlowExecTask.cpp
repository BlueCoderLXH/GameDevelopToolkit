//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"


void UFlowExecTask::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
    Output.ErrorMessage = "Not Implemented";
    Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
}

#if WITH_EDITOR
FLinearColor UFlowExecTask::GetNodeColor() const {
    return FLinearColor(0.08f, 0.08f, 0.08f);
}
#endif // WITH_EDITOR

////////////////////////////// FFlowExecNodeState //////////////////////////////

FFlowExecNodeStatePtr FFlowExecNodeState::Clone() {
    FFlowExecNodeStatePtr Copy = MakeShareable(new FFlowExecNodeState);
    for (auto& Entry : StateObjects) {
        const FName& ObjectID = Entry.Key;
        UObject* State = Entry.Value;
        if (!State) continue;
        
        UObject* ClonedState = NewObject<UObject>(State->GetOuter(), State->GetClass(), NAME_None, RF_NoFlags, State);
        if (ClonedState) {
            if (ClonedState->Implements<UFlowExecCloneableState>()) {
                // Copy the state over
                IFlowExecCloneableState* CloneableInterface = Cast<IFlowExecCloneableState>(ClonedState);
                CloneableInterface->CloneFromStateObject(State);
            }
            
            Copy->SetStateObject(ObjectID, ClonedState);
        }
    }
    return Copy;
}

UObject* FFlowExecNodeState::GetStateObject(const FName& InObjectID) const {
    UObject* const* SearchResult = StateObjects.Find(InObjectID);
    return SearchResult ? *SearchResult : nullptr; 
}

void FFlowExecNodeState::SetStateObject(const FName& InObjectID, UObject* InObject) {
    UObject*& StateRef = StateObjects.FindOrAdd(InObjectID);
    StateRef = InObject;
}

void FFlowExecNodeState::AddReferencedObjects(FReferenceCollector& Collector) {
    Collector.AddReferencedObjects(StateObjects);
}


////////////////////////////// UFlowExecStateCloneable //////////////////////////////

UFlowExecCloneableState::UFlowExecCloneableState(const class FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void IFlowExecCloneableState::CloneFromStateObject(UObject* SourceObject) {
}

