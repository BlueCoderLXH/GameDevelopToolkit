//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/Layout/GraphLayout.h"

class UEdGraphNode_GrammarNode;

struct FLinearGraphLayoutConfig {
    float InterDistanceX = 100;
    float InterDistanceY = 100;
};

typedef TSharedPtr<struct FLinearLayoutNode> FLinearLayoutNodePtr;

struct FLinearLayoutNode {
    TWeakObjectPtr<UEdGraphNode_GrammarNode> Node;
    int32 LocationX = 0;
    int32 LocationY = 0;
    TArray<FLinearLayoutNodePtr> OutgoingNodes;
    TArray<FLinearLayoutNodePtr> IncomingNodes;
};


class DUNGEONARCHITECTEDITOR_API FLinearGraphLayout : public IGraphLayout {
public:
    FLinearGraphLayout(const FLinearGraphLayoutConfig& InConfig) : Config(InConfig) {
    }

    virtual ~FLinearGraphLayout() {
    }

    virtual void PerformLayout(UEdGraph_Grammar* Graph) override;


private:
    FLinearGraphLayoutConfig Config;

};

