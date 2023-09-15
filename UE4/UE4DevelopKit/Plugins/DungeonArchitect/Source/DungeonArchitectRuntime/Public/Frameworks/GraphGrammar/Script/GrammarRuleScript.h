//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/Script/GrammarScriptGraph.h"
#include "GrammarRuleScript.generated.h"

class UGrammarNodeType;

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGrammarRuleScriptGraphNode : public UGrammarScriptGraphNode {
    GENERATED_BODY()
public:
    UPROPERTY()
    TWeakObjectPtr<UGrammarNodeType> TypeInfo;

    UPROPERTY()
    int32 Index;

    UPROPERTY()
    bool bDisplayIndex;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGrammarRuleScript : public UObject {
    GENERATED_UCLASS_BODY()

public:
#if WITH_EDITORONLY_DATA
    UPROPERTY()
    class UEdGraph* EdGraph;
#endif // WITH_EDITORONLY_DATA

    UPROPERTY(EditAnywhere, Category = Graph)
    float Weight = 1.0f;

    UPROPERTY()
    UGrammarScriptGraph* ScriptGraph;

public:
    static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
};

