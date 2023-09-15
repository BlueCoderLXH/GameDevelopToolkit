//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "DungeonGraphUtils.generated.h"

/** Action to add a node to the graph */
USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FDungeonSchemaAction_NewNode : public FEdGraphSchemaAction {
    GENERATED_USTRUCT_BODY();

    /** Template of node we want to create */
    UPROPERTY()
    class UEdGraphNode* NodeTemplate;


    FDungeonSchemaAction_NewNode()
        : FEdGraphSchemaAction()
          , NodeTemplate(nullptr) {
    }

    FDungeonSchemaAction_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip,
                                 const int32 InGrouping)
        : FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping)
          , NodeTemplate(nullptr) {
    }

    // FEdGraphSchemaAction interface
    virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, FVector2D Location, bool bSelectNewNode = true) override;
    virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, FVector2D Location, bool bSelectNewNode = true) override;
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FEdGraphSchemaAction interface

    template <typename NodeType>
    static NodeType* SpawnNodeFromTemplate(class UEdGraph* ParentGraph, NodeType* InTemplateNode,
                                           const FVector2D Location, bool bSelectNewNode = true) {
        FDungeonSchemaAction_NewNode Action;
        Action.NodeTemplate = InTemplateNode;

        return Cast<NodeType>(Action.PerformAction(ParentGraph, nullptr, Location, bSelectNewNode));
    }
};

class DUNGEONARCHITECTRUNTIME_API FDungeonGraphUtils {
public:
    template <typename T>
    static TArray<T*> GetConnectedNodes(UEdGraphPin* Pin) {
        TArray<T*> Result;
        for (UEdGraphPin* OtherPin : Pin->LinkedTo) {
            T* OtherNode = Cast<T>(OtherPin->GetOwningNode());
            if (OtherNode) {
                Result.Add(OtherNode);
            }
        }
        return Result;
    }

    template <typename T>
    static const T* FindInHierarchy(const UObject* Object) {
        while (Object != nullptr) {
            if (Object->IsA<T>()) {
                return Cast<const T>(Object);
            }
            Object = Object->GetOuter();
        }
        return nullptr;
    }

    template <typename T>
    static T* FindInHierarchy(UObject* Object) {
        while (Object != nullptr) {
            if (Object->IsA<T>()) {
                return Cast<T>(Object);
            }
            Object = Object->GetOuter();
        }
        return nullptr;
    }

};

class DUNGEONARCHITECTRUNTIME_API FDungeonSchemaUtils {
public:
    template <typename T>
    static void AddAction(FString Title, FString Tooltip, TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions,
                          UEdGraph* OwnerOfTemporaries, int32 Priority = 0) {
        const FText MenuDesc = FText::FromString(Title);
        const FText Category = FText::FromString(TEXT("Dungeon"));
        const FText TooltipText = FText::FromString(Tooltip);
        TSharedPtr<FDungeonSchemaAction_NewNode> NewActorNodeAction = AddNewNodeAction(OutActions, Category, MenuDesc, TooltipText, Priority);
        T* ActorNode = NewObject<T>(OwnerOfTemporaries);
        NewActorNodeAction->NodeTemplate = ActorNode;
    }

    static TSharedPtr<FDungeonSchemaAction_NewNode> AddNewNodeAction(
            TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, const FText& Category, const FText& MenuDesc, const FText& Tooltip, int32 Priority = 0);
    
    template <typename TAction>
    static void AddCustomAction(FString Title, FString Tooltip, TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions) {
        const FText MenuDesc = FText::FromString(Title);
        const FText Category = FText::FromString(TEXT("Dungeon"));
        const FText TooltipText = FText::FromString(Tooltip);
        TSharedPtr<TAction> NewAction = MakeShared<TAction>(Category, MenuDesc, TooltipText, 0);
        OutActions.Add(NewAction);
    }

};

