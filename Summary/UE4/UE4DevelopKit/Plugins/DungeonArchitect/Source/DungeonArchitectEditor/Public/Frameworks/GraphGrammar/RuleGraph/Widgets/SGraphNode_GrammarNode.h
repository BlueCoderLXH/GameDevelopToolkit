//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SGraphNode.h"

class UEdGraphNode_GrammarNode;
class SHorizontalBox;

class DUNGEONARCHITECTEDITOR_API SGraphNode_GrammarNode : public SGraphNode {
public:
    SLATE_BEGIN_ARGS(SGraphNode_GrammarNode) {}
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, UEdGraphNode_GrammarNode* InNode);

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    virtual void CreatePinWidgets() override;
    virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
    // End of SGraphNode interface

protected:
    FSlateColor GetBorderBackgroundColor() const;
    virtual const FSlateBrush* GetNameIcon() const;

protected:

    static FLinearColor InactiveStateColor;
    static FLinearColor ActiveStateColorDim;
    static FLinearColor ActiveStateColorBright;
    static FLinearColor ErrorColor;


};

