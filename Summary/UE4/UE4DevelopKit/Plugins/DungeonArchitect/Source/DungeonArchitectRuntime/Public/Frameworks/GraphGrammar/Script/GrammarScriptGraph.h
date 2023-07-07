//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GrammarScriptGraph.generated.h"

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGrammarScriptGraphNode : public UObject {
    GENERATED_BODY()

public:
    UGrammarScriptGraphNode();

public:
    UPROPERTY()
    FGuid NodeId;

    UPROPERTY()
    TArray<UGrammarScriptGraphNode*> OutgoingNodes;

    UPROPERTY()
    TArray<UGrammarScriptGraphNode*> IncomingNodes;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGrammarScriptGraph : public UObject {
    GENERATED_UCLASS_BODY()
public:
    UGrammarScriptGraphNode* FindRootNode() const;
    
public:
    UPROPERTY()
    TArray<UGrammarScriptGraphNode*> Nodes;

};

