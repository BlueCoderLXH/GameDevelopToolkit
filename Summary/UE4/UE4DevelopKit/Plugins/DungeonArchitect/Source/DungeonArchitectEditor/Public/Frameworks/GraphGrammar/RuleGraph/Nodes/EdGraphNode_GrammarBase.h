//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphNode_GrammarBase.generated.h"

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_GrammarBase : public UEdGraphNode {
    GENERATED_UCLASS_BODY()

public:
#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
    virtual void NodeConnectionListChanged() override;
#endif // WITH_EDITOR

    virtual UEdGraphPin* GetInputPin() const { return nullptr; }
    virtual UEdGraphPin* GetOutputPin() const { return nullptr; }

protected:
    UEdGraphPin* GetPin(const FGuid& PinId) const;
};

