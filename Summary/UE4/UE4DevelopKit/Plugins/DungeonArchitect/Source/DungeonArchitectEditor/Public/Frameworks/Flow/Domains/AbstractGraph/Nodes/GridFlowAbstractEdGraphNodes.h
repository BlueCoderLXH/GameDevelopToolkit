//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodeBase.h"
#include "GridFlowAbstractEdGraphNodes.generated.h"

class UFlowAbstractNode;

UCLASS()
class DUNGEONARCHITECTEDITOR_API UGridFlowAbstractEdGraphNode : public UGridFlowAbstractEdGraphNodeBase {
    GENERATED_BODY()

public:
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

public:
    TWeakObjectPtr<UFlowAbstractNode> ScriptNode;
};

