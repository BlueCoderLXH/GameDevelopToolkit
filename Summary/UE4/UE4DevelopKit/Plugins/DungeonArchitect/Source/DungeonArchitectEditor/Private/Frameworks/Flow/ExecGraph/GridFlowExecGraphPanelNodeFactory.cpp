//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/ExecGraph/GridFlowExecGraphPanelNodeFactory.h"

#include "Frameworks/Flow/ExecGraph/Nodes/GridFlowExecEdGraphNodeBase.h"
#include "Frameworks/Flow/ExecGraph/Nodes/GridFlowExecEdGraphNodes.h"
#include "Frameworks/Flow/ExecGraph/Widgets/SGraphNode_GridFlowExecNode.h"

TSharedPtr<class SGraphNode> FGridFlowExecGraphPanelNodeFactory::CreateNode(UEdGraphNode* Node) const {
    if (UGridFlowExecEdGraphNodeBase* ExecNode = Cast<UGridFlowExecEdGraphNodeBase>(Node)) {
        TSharedPtr<SGraphNode_GridFlowExecNode> SNode = SNew(SGraphNode_GridFlowExecNode, ExecNode);

        if (ExecNode->IsA<UGridFlowExecEdGraphNode_Result>()) {
            SNode->SetBorderColor(FLinearColor(0.08f, 0.16f, 0.08f));
        }
        return SNode;
    }
    return nullptr;
}

