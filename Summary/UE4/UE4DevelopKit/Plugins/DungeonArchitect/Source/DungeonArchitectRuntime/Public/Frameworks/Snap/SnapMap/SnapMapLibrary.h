//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Snap/Lib/SnapLibrary.h"
#include "Frameworks/Snap/SnapMap/SnapMapModuleDatabase.h"

class UGrammarScriptGraphNode;

//////////////////////////////////////// Graph Node Impl //////////////////////////////////////////////
class DUNGEONARCHITECTRUNTIME_API FSnapGraphGrammarNode final : public SnapLib::ISnapGraphNode {
public:
    explicit FSnapGraphGrammarNode(UGrammarScriptGraphNode* InGraphNode);
    
    virtual FGuid GetNodeID() const override;;
    virtual FName GetCategory() const override;
    virtual TArray<SnapLib::ISnapGraphNodePtr> GetOutgoingNodes(const FGuid& IncomingNodeId) const override;
    UGrammarScriptGraphNode* GetGraphNode() const;

private:
    TWeakObjectPtr<UGrammarScriptGraphNode> GraphNode;
};


//////////////////////////////////////// Module Database Adapter //////////////////////////////////////////////
class DUNGEONARCHITECTRUNTIME_API FSnapMapGraphModDBItemImpl
    : public SnapLib::IModuleDatabaseItem
{
public:
    FSnapMapGraphModDBItemImpl(const FSnapMapModuleDatabaseItem& InItem) : Item(InItem) {}
    FSnapMapModuleDatabaseItem GetItem() const { return Item; }
    virtual SnapLib::FModuleNodePtr CreateModuleNode(const FGuid& InNodeId) override;

    virtual FBox GetBounds() override { return Item.ModuleBounds; }
    virtual TSoftObjectPtr<UWorld> GetLevel() override { return Item.Level; }
    virtual TSoftObjectPtr<UWorld> GetLogicLevel() override { return Item.LogicLevel; }
    virtual FName GetCategory() override { return Item.Category; }
    
private:
    FSnapMapModuleDatabaseItem Item;
};

class DUNGEONARCHITECTRUNTIME_API FSnapMapModuleDatabaseImpl : public SnapLib::IModuleDatabase {
public:
    FSnapMapModuleDatabaseImpl(USnapMapModuleDatabase* ModuleDB);
};

