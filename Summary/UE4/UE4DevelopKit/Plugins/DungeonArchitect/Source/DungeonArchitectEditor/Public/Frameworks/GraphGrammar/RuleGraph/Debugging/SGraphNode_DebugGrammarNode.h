//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SGraphNode.h"

class UEdGraphNode_DebugGrammarNode;
class SHorizontalBox;

class DUNGEONARCHITECTEDITOR_API SGraphNode_DebugGrammarNode : public SGraphNode {
public:
    SLATE_BEGIN_ARGS(SGraphNode_DebugGrammarNode) {}
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, UEdGraphNode_DebugGrammarNode* InNode);

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    virtual void CreatePinWidgets() override;
    virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
    // End of SGraphNode interface

protected:
    FSlateColor GetBorderBackgroundColor() const;
    virtual const FSlateBrush* GetNameIcon() const;
    FText GetText_AssignedMap() const;

    FSlateColor GetStatusMessageColor() const;
    EVisibility GetStatusMessageVisibility() const;
    FText GetStatusMessageText() const;

protected:
    static FLinearColor InactiveStateColor;
    static FLinearColor ActiveStateColorDim;
    static FLinearColor ActiveStateColorBright;
    static FLinearColor ErrorColor;

    static FLinearColor CurrentNodeColor;
    static FLinearColor ProcessedNodeColor;
};

class UEdGraphNode_DebugGrammarDoorNode;

class DUNGEONARCHITECTEDITOR_API SGraphNode_DebugGrammarDoorNode : public SGraphNode {
public:
    SLATE_BEGIN_ARGS(SGraphNode_DebugGrammarDoorNode) {}
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, UEdGraphNode_DebugGrammarDoorNode* InNode);

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    // End of SGraphNode interface

    // SNodePanel::SNode interface
    virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;
    virtual void MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter) override;
    virtual bool RequiresSecondPassLayout() const override;
    virtual void PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& NodeToWidgetLookup) const override;
    // End of SNodePanel::SNode interface

    void PositionBetweenTwoNodes(const FGeometry& StartGeom, const FGeometry& EndGeom) const;
    FText GetDoorDataText() const;

    EVisibility IsNodeVisible() const;
};

