//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadData.h"
#include "Core/Editors/LaunchPad/Widgets/SLaunchPadPage.h"

class ILaunchPadDataSource;

class SLaunchPadPage_Details : public SLaunchPadPage {
public:
    SLATE_BEGIN_ARGS(SLaunchPadPage_Details) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& Args, const FLaunchPadPageLayout_Details& InData);
    virtual FString GetTitle() override;

private:
    FLaunchPadPageLayout_Details Data;
};

