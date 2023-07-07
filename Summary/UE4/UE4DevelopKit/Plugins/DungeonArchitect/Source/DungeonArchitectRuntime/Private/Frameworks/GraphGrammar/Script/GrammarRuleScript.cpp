//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Script/GrammarRuleScript.h"


///////////////////////////////// UFlowEdGraphScript /////////////////////////////////

UGrammarRuleScript::UGrammarRuleScript(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
}

void UGrammarRuleScript::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) {
    UGrammarRuleScript* This = CastChecked<UGrammarRuleScript>(InThis);

#if WITH_EDITORONLY_DATA
    Collector.AddReferencedObject(This->EdGraph, This);
#endif	// WITH_EDITORONLY_DATA

    Super::AddReferencedObjects(This, Collector);
}

