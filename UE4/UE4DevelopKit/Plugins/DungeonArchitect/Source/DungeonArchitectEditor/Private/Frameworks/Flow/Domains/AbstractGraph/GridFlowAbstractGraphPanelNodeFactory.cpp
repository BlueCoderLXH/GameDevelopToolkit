//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractGraphPanelNodeFactory.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodes.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Widgets/SGraphNode_GridFlowAbstractNode.h"

TSharedPtr<class SGraphNode> FGridFlowAbstractGraphPanelNodeFactory::CreateNode(UEdGraphNode* Node) const {
    if (UGridFlowAbstractEdGraphNode* AbstractNode = Cast<UGridFlowAbstractEdGraphNode>(Node)) {
        TSharedPtr<SGraphNode_GridFlowAbstractNode> SNode = SNew(SGraphNode_GridFlowAbstractNode, AbstractNode);
        return SNode;
    }

    return nullptr;
}

