//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/StackSystem.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractNode.h"
#include "FlowAbstractGraphPathUtils.generated.h"

class UFlowAbstractGraphBase;
class UFlowAbstractNode;
template<typename TNode> class TFlowAbstractGraphQuery; 
typedef TFlowAbstractGraphQuery<UFlowAbstractNode> FFlowAbstractGraphQuery;
typedef TSharedPtr<class IFlowAGNodeGroupGenerator> IFlowAGNodeGroupGeneratorPtr;
typedef TSharedPtr<class FFlowAbstractGraphConstraints> FFlowAbstractGraphConstraintsPtr;

UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UFlowLayoutNodeCreationConstraint : public UObject {
    GENERATED_BODY()
public:
    virtual bool CanCreateNodeAt(const UFlowAbstractNode* Node, int32 TotalPathLength, int32 CurrentPathPosition) { return true; }
};


struct FFlowAGStaticGrowthState {
    UFlowAbstractGraphBase* Graph = nullptr;
    FFlowAbstractGraphQuery* GraphQuery = nullptr;
    UFlowAbstractNode* HeadNode = nullptr;
    TArray<TWeakObjectPtr<UFlowAbstractNode>> SinkNodes;
    const FRandomStream* Random = nullptr;
    int32 MinPathSize = 0;
    int32 MaxPathSize = 0;
    FLinearColor NodeColor;
    FString PathName;
    FString StartNodePathNameOverride;
    FString EndNodePathNameOverride;
    IFlowAGNodeGroupGeneratorPtr NodeGroupGenerator;
    FFlowAbstractGraphConstraintsPtr GraphConstraint;
    TArray<TWeakObjectPtr<UObject>> TaskExtenders;
    TWeakObjectPtr<UFlowLayoutNodeCreationConstraint> NodeCreationConstraint;
};

struct FFlowAGGrowthState_PathItem {
    FGuid NodeId;
    FGuid PreviousNodeId;
};

struct FFlowAGGrowthState {
    TArray<FFlowAGGrowthState_PathItem> Path;
    TSet<FGuid> Visited;
    TArray<FFlowAbstractNodeGroup> NodeGroups;
    TWeakObjectPtr<UFlowAbstractNode> TailNode = nullptr;
};

class DUNGEONARCHITECTRUNTIME_API FFlowAbstractGraphPathUtils {
public:
    //static bool GrowPath(const UFlowAbstractNode* CurrentNode, const FFlowAGStaticGrowthState& StaticState, FFlowAGGrowthState& OutState);
    static void FinalizePath(const FFlowAGStaticGrowthState& StaticState, FFlowAGGrowthState& State);
    static UFlowAbstractNode* CreateMergedCompositeNode(FFlowAbstractGraphQuery& GraphQuery, const FFlowAbstractNodeGroup& NodeGroup);
};



template<typename TNode> class TFlowAbstractGraphQuery; 
typedef TFlowAbstractGraphQuery<class UFlowAbstractNode> FFlowAbstractGraphQuery;

struct DUNGEONARCHITECTRUNTIME_API FFlowAGPathNodeGroup  {
    bool bIsGroup = false;
    float Weight = 1.0f;
    TArray<FGuid> GroupNodes;        // The list of nodes that belong to this node
    TArray<FGuid> GroupEdgeNodes;     // The list of nodes on the edge of the group (so they can connect to other nodes)
};

class DUNGEONARCHITECTRUNTIME_API IFlowAGNodeGroupGenerator {
    public:
    virtual ~IFlowAGNodeGroupGenerator() {}
    virtual void Generate(const FFlowAbstractGraphQuery& InGraphQuery, const UFlowAbstractNode* InCurrentNode,
            const FRandomStream& InRandom, const TSet<FGuid>& InVisited, TArray<FFlowAGPathNodeGroup>& OutGroups) const = 0;
    
    /**
    * Heuristics on the minimum allowed node group size. This is used to check if we have enough nodes before growing
    */ 
    virtual int32 GetMinNodeGroupSize() const { return 1; }
};
typedef TSharedPtr<class IFlowAGNodeGroupGenerator> IFlowAGNodeGroupGeneratorPtr;

class DUNGEONARCHITECTRUNTIME_API FNullFlowAGNodeGroupGenerator : public IFlowAGNodeGroupGenerator {
public:
    virtual void Generate(const FFlowAbstractGraphQuery& InGraphQuery, const UFlowAbstractNode* InCurrentNode,
            const FRandomStream& InRandom, const TSet<FGuid>& InVisited, TArray<FFlowAGPathNodeGroup>& OutGroups) const override;
};


struct FFlowAGPathStackFrame {
    TWeakObjectPtr<const UFlowAbstractNode> CurrentNode;
    TWeakObjectPtr<const UFlowAbstractNode> IncomingNode;
    FFlowAGGrowthState State;
};

typedef TStackSystem<struct FFlowAGPathStackGrowthTask, FFlowAGPathStackFrame, FFlowAGStaticGrowthState, struct FFlowAGPathingSystemResult> FlowPathGrowthSystem;
typedef TSharedPtr<FlowPathGrowthSystem> FlowPathGrowthSystemPtr;

struct FFlowAGPathStackGrowthTask {
    static void Execute(const FFlowAGPathStackFrame& InFrameState, const FFlowAGStaticGrowthState& StaticState, FlowPathGrowthSystem& StackSystem);
};


struct FFlowAGPathingSystemResult {
    FFlowAGGrowthState State;
    FFlowAGStaticGrowthState StaticState;
};

/**
 * Maintains a list of growth systems and runs them in parallel till the first solution is found.
 * This also avoids a single solution to from getting stuck and taking a very long time,
 * as multiple paths are being explored at the same time
 */
class FFlowAGPathingSystem {
public:
    FFlowAGPathingSystem(int64 InMaxFramesToProcess) : MaxFramesToProcess(InMaxFramesToProcess) {}
    void RegisterGrowthSystem(const UFlowAbstractNode* StartNode, const FFlowAGStaticGrowthState& StaticState, const int32 Count = 1);
    void Execute(int32 InNumParallelSearches);
    
    FORCEINLINE bool FoundResult() const { return bFoundResult; }
    FORCEINLINE bool HasTimeout() const { return bTimeout; }
    FFlowAGPathingSystemResult GetResult() const {
        check(bFoundResult);
        return Result;
    }

private:
    void Execute(int32 StartIdx, int32 EndIdx);

private:
    TArray<FlowPathGrowthSystemPtr> GrowthSystems;
    bool bFoundResult = false;
    bool bTimeout = false;
    
    int64 FrameCounter = 0;
	int64 MaxFramesToProcess = 1e3;
    
    FFlowAGPathingSystemResult Result;
};

