//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/Layout/GraphLayout.h"

class UEdGraphNode_GrammarNode;

struct FLayeredGraphLayoutConfig {
    float DepthDistance = 150;
    float SiblingDistance = 100;
};

typedef TSharedPtr<struct FLayeredLayoutNode> FLayeredLayoutNodePtr;

struct FLayeredLayoutNode {
    TWeakObjectPtr<UEdGraphNode_GrammarNode> Node;
    TArray<FLayeredLayoutNodePtr> Children;
    FLayeredLayoutNodePtr Parent;

    TArray<FLayeredLayoutNodePtr> OutgoingNodes;
    TArray<FLayeredLayoutNodePtr> IncomingNodes;

    float X = 0;
    int32 Depth = 0;

    float Mod = 0;
};


class DUNGEONARCHITECTEDITOR_API FLayeredGraphLayout : public IGraphLayout {
public:
    FLayeredGraphLayout(const FLayeredGraphLayoutConfig& InConfig) : Config(InConfig) {
    }

    virtual ~FLayeredGraphLayout() {
    }

    virtual void PerformLayout(UEdGraph_Grammar* Graph) override;


private:
    FLayeredGraphLayoutConfig Config;

};

