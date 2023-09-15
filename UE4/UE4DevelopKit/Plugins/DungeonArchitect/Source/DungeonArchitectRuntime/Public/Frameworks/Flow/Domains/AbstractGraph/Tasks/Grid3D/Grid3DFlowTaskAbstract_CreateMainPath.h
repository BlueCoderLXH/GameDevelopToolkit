//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_CreateMainPath.h"
#include "Grid3DFlowTaskAbstract_CreateMainPath.generated.h"

class UGrid3DLayoutNodeCreationConstraint;

UCLASS(Meta = (AbstractTask, Title = "Create Main Path", Tooltip = "Create a main path with spawn and goal", MenuPriority = 1100))
class DUNGEONARCHITECTRUNTIME_API UGrid3DFlowTaskAbstract_CreateMainPath : public UFlowTaskAbstract_CreateMainPath {
    GENERATED_BODY()
public:

    /**
        Enable this to control where the nodes are created

        Variable Name: bUseNodeCreationConstraint
    */
    UPROPERTY(EditAnywhere, Category = "Node Constraints")
    bool bUseNodeCreationConstraint = false;

    /**
        Use a blueprint to control where the layout nodes are allowed to be created   
        This is useful if you want static entrances to your dungeon
        This parameter requires bUseNodeCreationConstraint flag to be set

        Variable Name: N/A
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, SimpleDisplay, Category = "Node Constraints", Meta=(EditCondition = "bUseNodeCreationConstraint"))
    UGrid3DLayoutNodeCreationConstraint* NodeCreationConstraint;

protected:
    virtual UFlowLayoutNodeCreationConstraint* GetNodeCreationConstraintLogic() const override;
};

