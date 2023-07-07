//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SGraphNode.h"

class UGridFlowExecEdGraphNodeBase;
class SHorizontalBox;

class DUNGEONARCHITECTEDITOR_API SGraphNode_GridFlowExecNode : public SGraphNode {
public:
    SLATE_BEGIN_ARGS(SGraphNode_GridFlowExecNode) {}
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, UGridFlowExecEdGraphNodeBase* InNode);

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    virtual void CreatePinWidgets() override;
    virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
    // End of SGraphNode interface

    void SetBorderColor(const FLinearColor& InNodeBorderColor);

    const FSlateBrush* GetBorderImage() const;

protected:
    FSlateColor GetBorderBackgroundColor() const;
    virtual const FSlateBrush* GetNameIcon() const;
    FText GetNodeDescriptionText() const;
    EVisibility GetNodeDescriptionVisibility() const;
    FText GetNodeErrorText() const;

protected:
    FLinearColor DefaultNodeBorderColor;
    TSharedPtr<SErrorText> ErrorText;
    TSharedPtr<IErrorReportingWidget> ErrorReporting;
};

