//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "GridFlowAbstractEdGraphNodeBase.generated.h"

class UEdGraphPin;

UCLASS()
class DUNGEONARCHITECTEDITOR_API UGridFlowAbstractEdGraphNodeBase : public UEdGraphNode {
    GENERATED_UCLASS_BODY()

public:
#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
    virtual void NodeConnectionListChanged() override;
    virtual void AllocateDefaultPins() override;
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
};

