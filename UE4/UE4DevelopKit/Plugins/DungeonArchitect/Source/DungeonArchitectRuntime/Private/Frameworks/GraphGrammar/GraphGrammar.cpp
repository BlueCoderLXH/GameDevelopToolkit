//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/GraphGrammar.h"


#define LOCTEXT_NAMESPACE "GraphGrammarProduction"

////////////////////// Graph Production Rule ///////////////////////////
UGraphGrammarProduction::UGraphGrammarProduction(const FObjectInitializer& ObjectInitializer) : Super(
    ObjectInitializer) {
    RuleName = LOCTEXT("DefaultRuleName", "Rule");

}

///////////////////////////// UGraphGrammar /////////////////////////////

UGraphGrammar::UGraphGrammar(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    WildcardType = ObjectInitializer.CreateDefaultSubobject<UGrammarNodeType>(this, "WildcardNodeType");
    WildcardType->TypeName = "*";
    WildcardType->bWildcard = true;
}

////////////////////// GrammarNodeType ///////////////////////////
UGrammarNodeType::UGrammarNodeType(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    TypeName = "T";
    Description = FText::FromString("Task node");
    bNonTerminal = true;
    bWildcard = false;
    
#if WITH_EDITORONLY_DATA
    NodeColor = FLinearColor(0.08f, 0.08f, 0.08f);
#endif // WITH_EDITORONLY_DATA
}

#undef LOCTEXT_NAMESPACE

