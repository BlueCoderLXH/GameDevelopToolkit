//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Script/GrammarExecutionScript.h"


////////////////////////// UFlowExecutionEdGraphScript ////////////////////////// 

UGrammarExecutionScript::UGrammarExecutionScript(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {

}

void UGrammarExecutionScript::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) {
    UGrammarExecutionScript* This = CastChecked<UGrammarExecutionScript>(InThis);

#if WITH_EDITORONLY_DATA
    Collector.AddReferencedObject(This->EdGraph, This);
#endif	// WITH_EDITORONLY_DATA

    Super::AddReferencedObjects(This, Collector);

}

