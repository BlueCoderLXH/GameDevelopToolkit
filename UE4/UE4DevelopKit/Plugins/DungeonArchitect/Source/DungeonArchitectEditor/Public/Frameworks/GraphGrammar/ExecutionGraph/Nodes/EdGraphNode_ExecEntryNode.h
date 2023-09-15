//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecNodeBase.h"
#include "EdGraphNode_ExecEntryNode.generated.h"

class UEdGraphPin;
class UGraphGrammarProduction;

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_ExecEntryNode : public UEdGraphNode_ExecNodeBase {
    GENERATED_UCLASS_BODY()
public:

#if WITH_EDITOR
    virtual void AllocateDefaultPins() override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
#endif //WITH_EDITOR

    void InitializeNode_Runtime();

    virtual UEdGraphPin* GetOutputPin() const override;

private:
    void CreateNewGuid_Runtime();
    void AllocateDefaultPins_Runtime();
    UEdGraphPin* CreatePin_Runtime(EEdGraphPinDirection Dir, const FName& InPinCategory, const FName& PinName,
                                   int32 Index = INDEX_NONE);

};

