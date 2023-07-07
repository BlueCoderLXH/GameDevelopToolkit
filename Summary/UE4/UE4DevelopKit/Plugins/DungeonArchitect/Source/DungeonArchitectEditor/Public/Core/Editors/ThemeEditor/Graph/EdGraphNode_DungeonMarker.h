//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonBase.h"
#include "EdGraphNode_DungeonMarker.generated.h"

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_DungeonMarker : public UEdGraphNode_DungeonBase {
    GENERATED_UCLASS_BODY()

    // Begin UEdGraphNode interface.
    virtual void AllocateDefaultPins() override;
    virtual FLinearColor GetNodeTitleColor() const override;
    virtual FText GetTooltipText() const override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual void DestroyNode() override;
    // End UEdGraphNode interface.

    // UObject interface
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
    virtual bool CanEditChange(const FProperty* InProperty) const override;
    // End of UObject interface

public:
    virtual UEdGraphPin* GetInputPin() const override { return nullptr; }
    virtual UEdGraphPin* GetOutputPin() const override { return Pins[0]; }
    virtual void AutowireNewNode(UEdGraphPin* FromPin) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FString MarkerName;

    UPROPERTY()
    bool bUserDefined;


    UPROPERTY()
    bool bBuilderEmittedMarker;
};

