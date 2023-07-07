//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlowAbstractItem.generated.h"

UENUM()
enum class EFlowGraphItemType : uint8 {
    Enemy,
    Entrance,
    Exit,
    Treasure,
    Key,
    Lock,
    Teleporter,
    Custom
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FGridFlowItemCustomInfo {
    GENERATED_BODY()

    FGridFlowItemCustomInfo() {
    }

    FGridFlowItemCustomInfo(const FString& InPreviewText, const FLinearColor& InPreviewTextColor,
                            const FLinearColor InPreviewBackgroundColor)
        : PreviewText(InPreviewText)
          , PreviewTextColor(InPreviewTextColor)
          , PreviewBackgroundColor(InPreviewBackgroundColor) {
    }

    UPROPERTY(EditAnywhere, Category="Dungeon")
    FString PreviewText = "X";

    UPROPERTY(EditAnywhere, Category = "Dungeon")
    FLinearColor PreviewTextColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, Category = "Dungeon")
    FLinearColor PreviewBackgroundColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, Category = "Dungeon")
    float FontScale = 1.0f;
};

UCLASS(BlueprintType)
class DUNGEONARCHITECTRUNTIME_API UFlowGraphItem : public UObject{
    GENERATED_BODY()
public:
    UFlowGraphItem();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon")
    FGuid ItemId = FGuid();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon")
    EFlowGraphItemType ItemType = EFlowGraphItemType::Enemy;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon")
    FString MarkerName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon")
    TArray<FGuid> ReferencedItemIds;

    UPROPERTY()
    bool bEditorSelected = false;

    UPROPERTY()
    FGridFlowItemCustomInfo CustomInfo;
};

class DUNGEONARCHITECTRUNTIME_API FGridFlowItemVisuals {
public:
    static FString GetText(const UFlowGraphItem* Item);
    static FLinearColor GetTextColor(const UFlowGraphItem* Item, bool bInvertColor = false);
    static FLinearColor GetBackgroundColor(const UFlowGraphItem* Item, bool bInvertColor = false);
};

struct DUNGEONARCHITECTRUNTIME_API FFlowGraphItemContainer {
    FGuid ItemId;
    TWeakObjectPtr<class UFlowAbstractNode> HostNode;
    TWeakObjectPtr<class UFlowAbstractLink> HostLink;

    UFlowGraphItem* GetItem() const;
    void* GetParentObject() const;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonFlowItemMetadataEvent, const UFlowGraphItem*, Item);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class DUNGEONARCHITECTRUNTIME_API UDungeonFlowItemMetadataComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon")
    TWeakObjectPtr<const UFlowGraphItem> FlowItem;
    
    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonFlowItemMetadataEvent OnFlowItemUpdated;

public:
    void SetFlowItem(TWeakObjectPtr<const UFlowGraphItem> InFlowItem);
};

