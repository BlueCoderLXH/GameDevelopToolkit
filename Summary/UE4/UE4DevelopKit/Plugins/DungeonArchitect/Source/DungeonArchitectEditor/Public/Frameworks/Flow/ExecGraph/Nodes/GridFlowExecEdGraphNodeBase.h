//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskStructs.h"

#include "EdGraph/EdGraphNode.h"
#include "GridFlowExecEdGraphNodeBase.generated.h"

class UEdGraphPin;

UCLASS()
class DUNGEONARCHITECTEDITOR_API UGridFlowExecEdGraphNodeBase : public UEdGraphNode {
    GENERATED_UCLASS_BODY()

public:
#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
    virtual void NodeConnectionListChanged() override;
    virtual void AllocateDefaultPins() override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
    virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
    UEdGraphPin* CreatePin(EEdGraphPinDirection Dir, const FName& InPinCategory, const FName& PinName,
                           int32 Index = INDEX_NONE);
#endif // WITH_EDITOR

public:
    virtual UEdGraphPin* GetInputPin() const;
    virtual UEdGraphPin* GetOutputPin() const;
    virtual void InitializeNode();

protected:
    UEdGraphPin* GetPin(const FGuid& PinId) const;

public:
    EFlowTaskExecutionStage ExecutionStage = EFlowTaskExecutionStage::NotExecuted;
    EFlowTaskExecutionResult ExecutionResult = EFlowTaskExecutionResult::FailHalt;
    FString ErrorMessage;
};

