//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/ExecGraph/Nodes/GridFlowExecEdGraphNodeBase.h"
#include "GridFlowExecEdGraphNodes.generated.h"

class UFlowExecTask;

UCLASS()
class DUNGEONARCHITECTEDITOR_API UGridFlowExecEdGraphNode_Task : public UGridFlowExecEdGraphNodeBase {
    GENERATED_BODY()

public:
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual bool CanPasteHere(const UEdGraph* TargetGraph) const override;
    
public:
    UPROPERTY()
    UFlowExecTask* TaskTemplate;
};

UCLASS()
class DUNGEONARCHITECTEDITOR_API UGridFlowExecEdGraphNode_Result : public UGridFlowExecEdGraphNodeBase {
    GENERATED_BODY()

public:
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual bool CanDuplicateNode() const override { return false; }
};

