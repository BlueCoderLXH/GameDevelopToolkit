//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"

#include "Frameworks/GraphGrammar/GrammarScriptCompiler.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/Layout/LayeredGraphLayout.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraphSchema_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"
#include "Frameworks/GraphGrammar/Script/GrammarRuleScript.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlowEdGraphScript, Log, All);

UEdGraph_Grammar::UEdGraph_Grammar(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Schema = UEdGraphSchema_Grammar::StaticClass();
}

UEdGraphNode_GrammarNode* UEdGraph_Grammar::AddNewNode(TWeakObjectPtr<UGrammarNodeType> InNodeType) {
    return AddNewNodeOfType<UEdGraphNode_GrammarNode>(InNodeType);
}


DUNGEONARCHITECTEDITOR_API void DACopyScriptNodeData(UGrammarRuleScriptGraphNode* ScriptNode,
                                                     UEdGraphNode_GrammarNode* RuleEdNode) {
    ScriptNode->TypeInfo = RuleEdNode->TypeInfo;
    ScriptNode->Index = RuleEdNode->Index;
    ScriptNode->bDisplayIndex = RuleEdNode->bDisplayIndex;
}

typedef TGrammarScriptCompiler<UEdGraphNode_GrammarNode, UGrammarRuleScriptGraphNode, UGrammarRuleScript>
FGrammarRuleScriptCompiler;

void UEdGraph_Grammar::NotifyGraphChanged() {
    Super::NotifyGraphChanged();

    UGrammarRuleScript* Script = Cast<UGrammarRuleScript>(GetOuter());
    FGrammarRuleScriptCompiler::Compile(Script);
}

void UEdGraph_Grammar::LayoutGraph(int32 HorizontalSpacing, int32 VerticalSpacing) {
    FLayeredGraphLayoutConfig Config;
    Config.DepthDistance = HorizontalSpacing;
    Config.SiblingDistance = VerticalSpacing;
    TSharedPtr<IGraphLayout> GraphLayout = MakeShareable(new FLayeredGraphLayout(Config));
    GraphLayout->PerformLayout(this);
}

