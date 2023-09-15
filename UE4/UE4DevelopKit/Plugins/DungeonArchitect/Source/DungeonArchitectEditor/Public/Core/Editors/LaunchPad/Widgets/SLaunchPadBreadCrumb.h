//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/LaunchPad/Widgets/SLaunchPadCommon.h"

#include "Widgets/SCompoundWidget.h"

class SLaunchPadPage;
class SVerticalBox;


class SLaunchPadBreadCrumb : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SLaunchPadBreadCrumb) {}
        SLATE_EVENT(FLaunchPadPageNavigation, OnNavigation);
    SLATE_END_ARGS()

    void Construct(const FArguments& Args);
    void SetRoot(TSharedPtr<SLaunchPadPage> InPage);
    void PushPage(TSharedPtr<SLaunchPadPage> InPage);

private:
    void UpdateWidget();
    void OnLinkClicked(TSharedPtr<SLaunchPadPage> InPage);

private:
    FLaunchPadPageNavigation OnNavigation;
    TArray<TSharedPtr<SLaunchPadPage>> PageStack;
    TSharedPtr<SHorizontalBox> BreadCrumbHost;
};

