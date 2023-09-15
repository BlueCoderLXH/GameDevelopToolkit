//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"
#include "FlowAbstractLink.generated.h"

UENUM()
enum class EFlowAbstractLinkType : uint8 {
    Unconnected,
    Connected,
    OneWay
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UFlowAbstractLink : public UObject {
    GENERATED_BODY()
    
public:
    UPROPERTY()
    FGuid LinkId;

    UPROPERTY()
    FGuid Source;

    UPROPERTY()
    FGuid Destination;

    UPROPERTY()
    EFlowAbstractLinkType Type = EFlowAbstractLinkType::Unconnected;

    UPROPERTY()
    TArray<UFlowGraphItem*> LinkItems;

    /** If the source node was merged, the original unmerged node id would be here */
    UPROPERTY()
    FGuid SourceSubNode;
    
    /** If the destination node was merged, the original unmerged node id would be here */
    UPROPERTY()
    FGuid DestinationSubNode;

public:
    virtual UFlowAbstractLink* Clone(UObject* Outer);
    virtual void ReverseDirection();
    
    template<typename TItem>
    TItem* CreateNewItem() {
        TItem* NewItem = NewObject<TItem>(this, TItem::StaticClass());
        LinkItems.Add(NewItem);
        return NewItem;
    }

};

