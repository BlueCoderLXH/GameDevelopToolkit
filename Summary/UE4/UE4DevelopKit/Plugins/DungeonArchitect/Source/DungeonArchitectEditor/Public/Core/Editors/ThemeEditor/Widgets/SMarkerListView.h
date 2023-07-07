//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableRow.h"

class UEdGraphNode_DungeonMarker;

struct FMarkerListEntry {
    FString MarkerName;
    TWeakObjectPtr<UEdGraphNode_DungeonMarker> MarkerNode;
};

DECLARE_DELEGATE_OneParam(FOnMarkerItemDoubleClicked, TSharedPtr<FMarkerListEntry>);

class SMarkerListView : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SMarkerListView) {}
        SLATE_EVENT(FOnMarkerItemDoubleClicked, OnMarkerDoubleClicked)
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs);
    void Refresh(TArray<TSharedPtr<FMarkerListEntry>> InMarkerListEntries);

private:
    TSharedRef<ITableRow> OnGenerateMarkerListRow(TSharedPtr<FMarkerListEntry> InItem,
                                                  const TSharedRef<STableViewBase>& OwnerTable);
    void OnMarkerListDoubleClicked(TSharedPtr<FMarkerListEntry> Entry);
    void OnFilterTextChanged(const FText& InFilterText);
    void OnFilterTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType);
    void RefreshImpl();

private:
    FOnMarkerItemDoubleClicked OnMarkerDoubleClicked;
    TArray<TSharedPtr<FMarkerListEntry>> MarkerListEntries;
    TArray<TSharedPtr<FMarkerListEntry>> FilteredMarkerListEntries;

    typedef SListView<TSharedPtr<FMarkerListEntry>> FMarkerListView;
    TSharedPtr<FMarkerListView> MarkersListView;
    FText FilterText;
};

