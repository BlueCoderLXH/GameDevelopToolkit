//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Framework/SlateDelegates.h"
#include "Widgets/SCompoundWidget.h"

class SHelpSystemList : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SHelpSystemList) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& Args);

private:
    TSharedPtr<SWidget> CreateEntry(const FText& Title, const FText& Description, const FName& IconId,
                                    FOnClicked OnClicked);

};

