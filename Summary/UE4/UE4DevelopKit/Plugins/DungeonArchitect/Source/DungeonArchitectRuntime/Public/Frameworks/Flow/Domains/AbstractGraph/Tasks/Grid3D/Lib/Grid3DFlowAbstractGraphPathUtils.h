//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/Lib/FlowAbstractGraphPathUtils.h"
#include "Grid3DFlowAbstractGraphPathUtils.generated.h"

UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UGrid3DLayoutNodeCreationConstraint : public UFlowLayoutNodeCreationConstraint {
    GENERATED_BODY()
    public:
    UFUNCTION(BlueprintNativeEvent, Category="Dungeon")
    bool CanCreateLayoutNode(const FIntVector& NodeCoord, const FIntVector& GridSize, int32 TotalPathLength, int32 CurrentPathPosition);
    virtual bool CanCreateLayoutNode_Implementation(const FIntVector& NodeCoord, const FIntVector& GridSize, int32 TotalPathLength, int32 CurrentPathPosition);

    virtual bool CanCreateNodeAt(const UFlowAbstractNode* Node, int32 TotalPathLength, int32 CurrentPathPosition) override;
};

