//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FLaunchPadCategoryItem;
class SLaunchPadCategories;
class SLaunchPadPage;
class SLaunchPadBreadCrumb;
class SBox;
typedef TSharedPtr<class ILaunchPadDataSource> ILaunchPadDataSourcePtr;

class SLaunchPad : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SLaunchPad) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& Args);

private:
    void OnCategorySelectionChanged(TSharedPtr<FLaunchPadCategoryItem> SelectedItem, ESelectInfo::Type SelectInfo);
    void OnBreadCrumbNavigation(TSharedPtr<SLaunchPadPage> InPage);
    TSharedPtr<SLaunchPadPage> CreatePage(const FString& InPath);
    void OnLinkClicked(const FString& InPath);

    TSharedPtr<SWidget> CreateSidebarLogo() const;

private:
    ILaunchPadDataSourcePtr DataSource;
    TSharedPtr<SLaunchPadCategories> Categories;
    TSharedPtr<SBox> PageHost;
    TSharedPtr<SLaunchPadBreadCrumb> BreadCrumb;
};

