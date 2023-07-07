//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarker.h"

#include "SGraphNode.h"

/**
 * Implements the message interaction graph panel.
 */
class DUNGEONARCHITECTEDITOR_API SGraphNode_DungeonMarker : public SGraphNode, public FNodePropertyObserver {
public:
    SLATE_BEGIN_ARGS(SGraphNode_DungeonMarker) {}
    SLATE_END_ARGS()

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, UEdGraphNode_DungeonMarker* InNode);

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

    FString GetNodeErrorText() const;

protected:
    FSlateColor GetBorderBackgroundColor() const;
    virtual const FSlateBrush* GetNameIcon() const;
    bool IsNameDuplicate(const FString& Name) const;
    bool IsNodeUserDefined() const;
    virtual bool IsNameReadOnly() const override;

protected:
    TSharedPtr<SHorizontalBox> OutputPinBox;

    static FLinearColor InactiveStateColor;
    static FLinearColor ActiveStateColorDim;
    static FLinearColor ActiveStateColorBright;


};

