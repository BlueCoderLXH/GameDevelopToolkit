//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class SWidget;
class FEdMode;


class SSnapMapEditor : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SSnapMapEditor) {}
    SLATE_END_ARGS()


public:
    /** SCompoundWidget functions */
    void Construct(const FArguments& InArgs);

    void SetSettingsObject(UObject* Object, bool bForceRefresh = false);

private:
    TSharedPtr<class IDetailsView> DetailsPanel;
};

