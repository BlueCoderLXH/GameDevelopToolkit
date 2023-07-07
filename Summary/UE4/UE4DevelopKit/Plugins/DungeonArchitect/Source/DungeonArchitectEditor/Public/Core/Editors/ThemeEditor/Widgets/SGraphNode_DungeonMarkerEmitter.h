//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarkerEmitter.h"

#include "SGraphNode.h"

/**
 * Implements the message interaction graph panel.
 */
class DUNGEONARCHITECTEDITOR_API SGraphNode_DungeonMarkerEmitter : public SGraphNode, public FNodePropertyObserver {
public:
    SLATE_BEGIN_ARGS(SGraphNode_DungeonMarkerEmitter) {}
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, UEdGraphNode_DungeonMarkerEmitter* InNode);

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    virtual void CreatePinWidgets() override;
    virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
    // End of SGraphNode interface

    // FPropertyObserver interface
    virtual void OnPropertyChanged(UEdGraphNode_DungeonBase* Sender, const FName& PropertyName) override;
    // End of FPropertyObserver interface

    /* Called when text is committed on the node */
    void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);

    /** Called when text is being committed to check for validity */
    bool OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage);

protected:
    FSlateColor GetBorderBackgroundColor() const;
    virtual const FSlateBrush* GetNameIcon() const;
    virtual bool IsNameReadOnly() const override;

protected:
    TSharedPtr<SHorizontalBox> OutputPinBox;

    static FLinearColor InactiveStateColor;
    static FLinearColor ActiveStateColorDim;
    static FLinearColor ActiveStateColorBright;
};

