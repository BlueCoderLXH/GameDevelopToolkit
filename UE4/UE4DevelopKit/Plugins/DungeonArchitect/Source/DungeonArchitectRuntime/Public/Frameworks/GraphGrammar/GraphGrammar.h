//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GraphGrammar.generated.h"

class UGrammarRuleScript;
class UGrammarExecutionScript;

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGraphGrammarProduction : public UObject {
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Rule")
    FText RuleName;

    UPROPERTY()
    UGrammarRuleScript* SourceGraph;

    UPROPERTY()
    TArray<UGrammarRuleScript*> DestGraphs;

};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGrammarNodeType : public UObject {
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "NodeType")
    FName TypeName;

    UPROPERTY(EditAnywhere, Category = "NodeType")
    FText Description;

    UPROPERTY(EditAnywhere, Category = "NodeType")
    bool bNonTerminal;

    UPROPERTY()
    bool bWildcard;

#if WITH_EDITORONLY_DATA
    UPROPERTY(EditAnywhere, Category = "NodeType")
    FLinearColor NodeColor;
#endif // WITH_EDITORONLY_DATA
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGraphGrammar : public UObject {
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY()
    TArray<UGraphGrammarProduction*> ProductionRules;

    UPROPERTY()
    UGrammarExecutionScript* ExecutionGraphScript;

    UPROPERTY()
    TArray<UGrammarNodeType*> NodeTypes;

    UPROPERTY()
    UGrammarNodeType* WildcardType;

};

