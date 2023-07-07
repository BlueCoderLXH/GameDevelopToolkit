//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphConstraints.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstractBase.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/Lib/FlowAbstractGraphPathUtils.h"
#include "SnapFlowAbstractGraphSupport.generated.h"

struct FFlowAGPathNodeGroup;
class USnapGridFlowModuleDatabase;

//////////////////////////////////////// Snap Abstract Graph Node Group Generator //////////////////////////////////////////////

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FSnapFlowAGNodeGroupSetting {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Node Group")  
    float Weight = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Node Group")  
    FIntVector GroupSize = FIntVector(1, 1, 1);
};

class DUNGEONARCHITECTRUNTIME_API FSnapFlowAGNodeGroupGenerator : public IFlowAGNodeGroupGenerator {
public:
    explicit FSnapFlowAGNodeGroupGenerator(const USnapGridFlowModuleDatabase* ModuleDB);
    virtual void Generate(const FFlowAbstractGraphQuery& InGraphQuery, const UFlowAbstractNode* InCurrentNode,
            const FRandomStream& InRandom, const TSet<FGuid>& InVisited, TArray<FFlowAGPathNodeGroup>& OutGroups) const override;
    
    virtual int32 GetMinNodeGroupSize() const override;

private:
    TArray<FSnapFlowAGNodeGroupSetting> GroupSettings;
};


//////////////////////////////////////// Snap Abstract Graph Constraint System //////////////////////////////////////////////

class DUNGEONARCHITECTRUNTIME_API FSnapGridFlowAbstractGraphConstraints : public FFlowAbstractGraphConstraints {
public:
    FSnapGridFlowAbstractGraphConstraints(USnapGridFlowModuleDatabase* InModuleDatabase);
    virtual bool IsValid(const FFlowAbstractGraphQuery& InGraphQuery, const UFlowAbstractNode* Node, const TArray<const UFlowAbstractNode*>& IncomingNodes, const TArray<TWeakObjectPtr<UObject>>& InTaskExtenders) override;
    virtual bool IsValid(const FFlowAbstractGraphQuery& InGraphQuery, const FFlowAGPathNodeGroup& Group, int32 PathIndex, int32 PathLength, const TArray<FFAGConstraintsLink>& IncomingNodes, const TArray<TWeakObjectPtr<UObject>>& InTaskExtenders) override;
    static void BuildNodeGroup(const FFlowAbstractGraphQuery& InGraphQuery, const UFlowAbstractNode* InNode, const TArray<const UFlowAbstractNode*>& InIncomingNodes, FFlowAGPathNodeGroup& OutGroup, TArray<FFAGConstraintsLink>& OutConstraintLinks);

private:
    bool IsValid(const FFlowAbstractGraphQuery& InGraphQuery, const FFlowAGPathNodeGroup& Group, const TArray<FFAGConstraintsLink>& IncomingNodes, const TArray<FName>& InAllowedCategories) const;
    
private:
    TWeakObjectPtr<USnapGridFlowModuleDatabase> ModuleDatabase;
};


//////////////////////////////////////// Snap Abstract Graph Domain Data //////////////////////////////////////////////

/**
    Tilemap domain specific data that is attached to the abstract graph nodes
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UFANodeSnapDomainData : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FName> ModuleCategories;
};

//////////////////////////////////////// Snap Abstract Graph Task Extender /////////////////////////////////////////////

UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, HideDropdown)
class USnapFlowNodeCategorySelectionOverride : public UObject {
    GENERATED_BODY()
public:
    /** Change the category list if needed. Return true if the new list should be used, false to ignore this override blueprint */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    bool TryOverrideCategories(int32 PathIndex, int32 PathLength, const TArray<FName>& ExistingCategories, TArray<FName>& OutNewCategories);
    virtual bool TryOverrideCategories_Implementation(int32 PathIndex, int32 PathLength, const TArray<FName>& ExistingCategories, TArray<FName>& OutNewCategories) { return false; }
};

UENUM()
enum class ESnapFlowAGTaskModuleCategoryOverrideMethod : uint8 {
    None        UMETA(DisplayName = "None"),
    StartEnd   UMETA(DisplayName = "Start / End Nodes"),
    Blueprint   UMETA(DisplayName = "Blueprint"),
};


UCLASS()
class DUNGEONARCHITECTRUNTIME_API USnapFlowAGTaskExtender : public UFlowAbstractGraphTaskExtender {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Snap")
    TArray<FName> ModuleCategories = { "Room" };

    UPROPERTY(EditAnywhere, Category="Snap")
    ESnapFlowAGTaskModuleCategoryOverrideMethod ModuleCategoryOverrideMethod = ESnapFlowAGTaskModuleCategoryOverrideMethod::None;
    
    UPROPERTY(EditAnywhere, Category="Snap", Meta=(EditCondition="ModuleCategoryOverrideMethod == ESnapFlowAGTaskModuleCategoryOverrideMethod::StartEnd"))
    TArray<FName> StartNodeCategoryOverride;
    
    UPROPERTY(EditAnywhere, Category="Snap", Meta=(EditCondition="ModuleCategoryOverrideMethod == ESnapFlowAGTaskModuleCategoryOverrideMethod::StartEnd"))
    TArray<FName> EndNodeCategoryOverride;
    
    /**
     * Use a blueprint logic to change the category list on a per-node basis, if needed.
     * Create a blueprint inherited from "Snap Flow Node Category Selection Override". Then override the "TryOverrideCategories" function
     */
    UPROPERTY(EditAnywhere, Instanced, SimpleDisplay, Category="Snap", Meta=(EditCondition="ModuleCategoryOverrideMethod == ESnapFlowAGTaskModuleCategoryOverrideMethod::Blueprint"))
    TArray<USnapFlowNodeCategorySelectionOverride*> CategoryOverrideLogic;
    
public:
    virtual void ExtendNode(UFlowAbstractNode* Node) override;
    TArray<FName> GetCategoriesAtNode(int32 PathIndex, int32 PathLength);

#if WITH_EDITOR
    virtual FString GetDetailsPanelCategoryName() const override;
#endif //WITH_EDITOR

};



