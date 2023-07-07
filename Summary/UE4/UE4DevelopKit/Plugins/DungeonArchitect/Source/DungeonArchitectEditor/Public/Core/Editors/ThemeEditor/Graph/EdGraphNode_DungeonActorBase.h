//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonSpatialConstraint.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonBase.h"
#include "Frameworks/ThemeEngine/Rules/DungeonSelectorLogic.h"
#include "Frameworks/ThemeEngine/Rules/DungeonSpawnLogic.h"
#include "Frameworks/ThemeEngine/Rules/DungeonTransformLogic.h"
#include "EdGraphNode_DungeonActorBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(DungeonNodeLog, Log, All);

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_DungeonActorBase : public UEdGraphNode_DungeonBase {
    GENERATED_UCLASS_BODY()

    // Begin UEdGraphNode interface.
    virtual void AllocateDefaultPins() override;
    virtual FLinearColor GetNodeTitleColor() const override;
    virtual FText GetTooltipText() const override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    // End UEdGraphNode interface.

    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;

public:
    virtual UEdGraphPin* GetInputPin() const override { return Pins[0]; }
    virtual UEdGraphPin* GetOutputPin() const override { return Pins[1]; }
    virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
    virtual void NodeConnectionListChanged() override;
    virtual UObject* GetNodeAssetObject(UObject* Outer) { return nullptr; }
    virtual UObject* GetThumbnailAssetObject();

public:
    /**
      The probability that this node would be processed.   
      A value of 1 would always process this node and spawn a mesh.  
      A probability of 0.5 would spawn this mesh 50% of the time
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float Probability;

    /** Deprecated. Use Probability instead */
    UPROPERTY()
    float Affinity;

    /**
      You can have multiple mesh nodes attached to the same marker type.
      Each node is processed in order from left to right until all of them
      are processed or a node has this flag checked. 
      This lets you attach multiple meshes / actors in the same marker point
      (with possible varying probabilities)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool ConsumeOnAttach;

    /** Use this to adjust the mesh pivot, rotation and scale of your mesh, when it is attached to the marker */
    UPROPERTY(BlueprintReadWrite, Category = Dungeon)
    FTransform Offset;

    /** Use blueprint based selection logic */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Selection Logic")
    bool bUseSelectionLogic;

    /** 
      Determines if we need to multiply the Affinity with the selection logic's result
      If true, the selection logic fully controls the outcome of the selection process, i.e either select(1) or deselect(0)  
      If false, then the Affinity would be multiplied with the selector's result
      Uncheck this if you also want the Affinity to be used along with your selection logic
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Selection Logic")
    bool bLogicOverridesAffinity;

    /**
    Create a blueprint based selection logic by clicking the plus sign.
    Override the SelectNode function in the blueprint to decide if
    this node is inserted into the scene based on your blueprint logic
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, SimpleDisplay, Category = "Selection Logic")
    TArray<UDungeonSelectorLogic*> SelectionLogics;

    /** Use blueprint based selection logic */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Transform Logic")
    bool bUseTransformLogic;

    /** Create a blueprint to decide on the transform offset that needs to be applied on this node */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Instanced, SimpleDisplay, Category = "Transform Logic")
    TArray<UDungeonTransformLogic*> TransformLogics;

    /** Use blueprint based selection logic */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn Logic")
    bool bUseSpawnLogic;

    /** Create a blueprint to decide on the transform offset that needs to be applied on this node */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Instanced, SimpleDisplay, Category = "Spawn Logic")
    TArray<UDungeonSpawnLogic*> SpawnLogics;

    /** Use spatial constraint based selection logic */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spatial Constraint")
    bool bUseSpatialConstraint;

    /** Apply spatial constraint information to selectively execute the node if it satisfies a condition based on the surroundings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Constraint")
    UDungeonSpatialConstraint* SpatialConstraint;

    UPROPERTY()
    int32 ExecutionOrder;
};

