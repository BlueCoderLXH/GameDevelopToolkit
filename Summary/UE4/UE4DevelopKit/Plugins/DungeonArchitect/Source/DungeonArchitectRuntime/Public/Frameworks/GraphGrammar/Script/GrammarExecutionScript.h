//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/Script/GrammarScriptGraph.h"
#include "GrammarExecutionScript.generated.h"

UENUM()
enum class ERuleNodeExecutionMode : uint8 {
    RunOnce UMETA(DisplayName = "Run Once"),
    RunWithProbability UMETA(DisplayName = "Run with Probability"),
    Iterate UMETA(DisplayName = "Iterate N times"),
    IterateRange UMETA(DisplayName = "Iterate anywhere between N-M times")
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FRuleNodeExecutionModeConfig {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, Category = Advanced)
    float RunProbability = 0.5f;

    UPROPERTY(EditAnywhere, Category = Advanced)
    int32 IterationCount = 3;

    UPROPERTY(EditAnywhere, Category = Advanced)
    int32 IterationCountMin = 3;

    UPROPERTY(EditAnywhere, Category = Advanced)
    int32 IterationCountMax = 5;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGrammarExecutionScriptEntryNode : public UGrammarScriptGraphNode {
    GENERATED_BODY()
public:

};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGrammarExecutionScriptRuleNode : public UGrammarScriptGraphNode {
    GENERATED_BODY()
public:
    UPROPERTY()
    TWeakObjectPtr<class UGraphGrammarProduction> Rule;

    UPROPERTY()
    ERuleNodeExecutionMode ExecutionMode;

    UPROPERTY()
    FRuleNodeExecutionModeConfig ExecutionConfig;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGrammarExecutionScript : public UObject {
    GENERATED_UCLASS_BODY()

public:
#if WITH_EDITORONLY_DATA
    UPROPERTY()
    class UEdGraph* EdGraph;
#endif // WITH_EDITORONLY_DATA

    UPROPERTY()
    UGrammarScriptGraph* ScriptGraph;

    UPROPERTY()
    UGrammarExecutionScriptEntryNode* EntryNode;

public:
    static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
};

