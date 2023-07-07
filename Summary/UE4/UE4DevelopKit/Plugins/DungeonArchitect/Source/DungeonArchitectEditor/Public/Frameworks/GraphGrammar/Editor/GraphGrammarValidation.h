//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

enum class EGrammarLogType : uint8 {
    Warning,
    Error,
    Info,
    Success
};

typedef TSharedPtr<class IGrammarFocusAction> IGrammarFocusActionPtr;

struct FGrammarValidationEntry {
    FGrammarValidationEntry(EGrammarLogType InLogType, FText InMessage, IGrammarFocusActionPtr InFocusAction)
        : LogType(InLogType)
          , Message(InMessage)
          , FocusAction(InFocusAction) {
    }

    EGrammarLogType LogType;
    FText Message;
    IGrammarFocusActionPtr FocusAction;
};

typedef TSharedPtr<struct FGrammarValidationEntry> FGrammarValidationEntryPtr;
typedef TSharedRef<struct FGrammarValidationEntry> FGrammarValidationEntryRef;

// Contains the result of the graph validation.  It will contain entries if there are any errors / warnings
class FGrammarValidationResult {
public:
    void Finalize() {
        NumErrors = 0;
        NumWarnings = 0;
        for (FGrammarValidationEntryPtr Entry : Entries) {
            if (Entry->LogType == EGrammarLogType::Error) {
                NumErrors++;
            }
            else if (Entry->LogType == EGrammarLogType::Warning) {
                NumWarnings++;
            }
        }
    }

    bool ContainsErrors() const { return NumErrors > 0; }
    bool ContainsWarnings() const { return NumWarnings > 0; }
    int32 GetNumErrors() const { return NumErrors; }
    int32 GetNumWarnings() const { return NumWarnings; }

public:
    TArray<FGrammarValidationEntryPtr> Entries;

private:
    int32 NumErrors;
    int32 NumWarnings;
};

class UEdGraph_Grammar;
class UGrammarNodeType;
class UGraphGrammar;
class UGraphGrammarProduction;

class FGrammarValidator {
public:
    static FGrammarValidationResult Validate(UGraphGrammar* Grammar);

private:
    static void ValidateBasics(UGraphGrammar* Grammar, FGrammarValidationResult& OutResult);
    static void ValidateNodeTypes(UGraphGrammar* Grammar, FGrammarValidationResult& OutResult);
    static void ValidateGraphs(UGraphGrammar* Grammar, FGrammarValidationResult& OutResult);
    static void ValidateGraph(UGraphGrammarProduction* Rule, UEdGraph_Grammar* Graph,
                              FGrammarValidationResult& OutResult);
    static void ValidateGraphConnectivity(UGraphGrammarProduction* Rule, UEdGraph_Grammar* Graph,
                                          FGrammarValidationResult& OutResult);
};

