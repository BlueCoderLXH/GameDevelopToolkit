//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadData.h"

#include "Widgets/SCompoundWidget.h"

class SLaunchPadAction : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SLaunchPadAction) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const FLaunchPadPageActionData& InAction);

protected:
    FReply OnButtonClicked();
    FText GetTitle() const;
    FName GetBrushName() const;
    FOptionalSize GetButtonWidth() const;

private:
    FLaunchPadPageActionData Action;
};

