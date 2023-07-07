//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "SNodePanel.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SGraphPin;
class UEdGraphNode_ExecEntryNode;

class SGraphNode_ExecEntryNode : public SGraphNode {
public:
    SLATE_BEGIN_ARGS(SGraphNode_ExecEntryNode) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphNode_ExecEntryNode* InNode);

    // SNodePanel::SNode interface
    virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;
    // End of SNodePanel::SNode interface

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
    // End of SGraphNode interface


protected:
    FSlateColor GetBorderBackgroundColor() const;

    FText GetPreviewCornerText() const;
};

