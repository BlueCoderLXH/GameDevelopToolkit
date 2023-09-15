//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/ExecGraph/GridFlowExecEdGraph.h"

#include "Frameworks/Flow/ExecGraph/FlowExecGraphScript.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "Frameworks/Flow/ExecGraph/GridFlowExecEdGraphSchema.h"
#include "Frameworks/Flow/ExecGraph/Nodes/GridFlowExecEdGraphNodes.h"

#include "EdGraph/EdGraphPin.h"

////////////////////////// UEdGraph_FlowExec //////////////////////////

UGridFlowExecEdGraph::UGridFlowExecEdGraph(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Schema = UGridFlowExecEdGraphSchema::StaticClass();

    ResultNode = ObjectInitializer.CreateDefaultSubobject<UGridFlowExecEdGraphNode_Result>(this, "ResultNode");
    ResultNode->InitializeNode();
    AddNode(ResultNode);
}

UGridFlowExecEdGraphNode_Task* UGridFlowExecEdGraph::AddNewNode(UFlowExecTask* InTask) {
    UGridFlowExecEdGraphNode_Task* Node = NewObject<UGridFlowExecEdGraphNode_Task>(this);
    Node->InitializeNode();
    Node->TaskTemplate = InTask;
    AddNode(Node);
    return Node;
}


void UGridFlowExecEdGraph::NotifyGraphChanged() {
    Super::NotifyGraphChanged();

    UGridFlowExecScript* Script = Cast<UGridFlowExecScript>(GetOuter());
    FGridFlowExecScriptCompiler::Compile(this, Script);

}

////////////////////////// FGridFlowExecScriptCompiler //////////////////////////

void FGridFlowExecScriptCompiler::Compile(UEdGraph* EdGraph, UGridFlowExecScript* Script) {
    if (!EdGraph || !Script) {
        return;
    }

    // Clear out the old state
    Script->ScriptGraph = nullptr;
    Script->ResultNode = nullptr;

    // Create a new script graph that mirrors the ed graph
    UGridFlowExecScriptGraph* ScriptGraph = NewObject<UGridFlowExecScriptGraph>(Script);

    // Create the nodes
    TMap<UGridFlowExecEdGraphNodeBase*, UGridFlowExecScriptGraphNode*> EdToScriptNodes;
    for (UEdGraphNode* EdNode : EdGraph->Nodes) {
        if (UGridFlowExecEdGraphNode_Result* ResultEdNode = Cast<UGridFlowExecEdGraphNode_Result>(EdNode)) {
            UGridFlowExecScriptResultNode* ResultScriptNode = NewObject<UGridFlowExecScriptResultNode>(ScriptGraph);
            ResultScriptNode->NodeId = ResultEdNode->NodeGuid;
            ScriptGraph->Nodes.Add(ResultScriptNode);
            EdToScriptNodes.Add(ResultEdNode, ResultScriptNode);
            Script->ResultNode = ResultScriptNode;
        }
        else if (UGridFlowExecEdGraphNode_Task* TaskEdNode = Cast<UGridFlowExecEdGraphNode_Task>(EdNode)) {
            UGridFlowExecScriptTaskNode* TaskScriptNode = NewObject<UGridFlowExecScriptTaskNode>(ScriptGraph);
            TaskScriptNode->NodeId = TaskEdNode->NodeGuid;
            UFlowExecTask* TaskTemplate = TaskEdNode->TaskTemplate;
            if (TaskTemplate) {
                TaskScriptNode->Task = NewObject<UFlowExecTask>(TaskScriptNode,
                                                                    TaskEdNode->TaskTemplate->GetClass(), NAME_None,
                                                                    RF_NoFlags, TaskEdNode->TaskTemplate);
                // Clone the Task Extenders
                TaskScriptNode->Task->Extenders.Reset();
                for (UFlowExecTaskExtender* EdTaskExtender : TaskEdNode->TaskTemplate->Extenders) {
                    UFlowExecTaskExtender* ScriptTaskExtender = NewObject<UFlowExecTaskExtender>(TaskScriptNode->Task,
                                EdTaskExtender->GetClass(), NAME_None, RF_NoFlags, EdTaskExtender);
                    TaskScriptNode->Task->Extenders.Add(ScriptTaskExtender);
                } 
            }
            ScriptGraph->Nodes.Add(TaskScriptNode);
            EdToScriptNodes.Add(TaskEdNode, TaskScriptNode);
        }
    }

    // Create the link mapping
    for (UEdGraphNode* EdNode : EdGraph->Nodes) {
        if (UGridFlowExecEdGraphNodeBase* RuleEdNode = Cast<UGridFlowExecEdGraphNodeBase>(EdNode)) {
            UGridFlowExecScriptGraphNode* ScriptNode = EdToScriptNodes[RuleEdNode];
            for (UEdGraphPin* OutgoingPin : RuleEdNode->GetOutputPin()->LinkedTo) {
                if (UGridFlowExecEdGraphNodeBase* OutgoingRuleEdNode = Cast<UGridFlowExecEdGraphNodeBase>(
                    OutgoingPin->GetOwningNode())) {
                    // Make an outgoing / incoming link
                    UGridFlowExecScriptGraphNode* OutgoingScriptNode = EdToScriptNodes[OutgoingRuleEdNode];
                    ScriptNode->OutgoingNodes.Add(OutgoingScriptNode);
                    OutgoingScriptNode->IncomingNodes.Add(ScriptNode);
                }
            }
        }
    }
    Script->ScriptGraph = ScriptGraph;
}

