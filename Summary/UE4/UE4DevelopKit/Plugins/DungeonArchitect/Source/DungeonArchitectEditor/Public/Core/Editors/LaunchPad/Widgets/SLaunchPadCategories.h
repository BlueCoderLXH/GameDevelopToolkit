//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Widgets/Views/SListView.h"

struct FLaunchPadCategoryItem;
typedef TSharedPtr<class ILaunchPadDataSource> ILaunchPadDataSourcePtr;

class SLaunchPadCategories : public SListView<TSharedPtr<FLaunchPadCategoryItem>> {
public:
    SLATE_BEGIN_ARGS(SLaunchPadCategories) {}
        SLATE_EVENT(FOnSelectionChanged, OnSelectionChanged)
    SLATE_END_ARGS()

    void Construct(const FArguments& Args);
    void Refresh(ILaunchPadDataSourcePtr InDataSource);

private:
    TSharedRef<class ITableRow> GenerateCategoryRowWidget(TSharedPtr<FLaunchPadCategoryItem> InItem,
                                                          const TSharedRef<class STableViewBase>& OwnerTable);

private:
    TArray<TSharedPtr<FLaunchPadCategoryItem>> Categories;
};

