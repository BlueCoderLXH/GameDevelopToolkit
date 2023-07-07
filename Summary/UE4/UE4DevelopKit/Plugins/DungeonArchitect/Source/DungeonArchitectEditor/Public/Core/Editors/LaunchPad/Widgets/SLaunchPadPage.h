//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/LaunchPad/Widgets/SLaunchPadCommon.h"

#include "Widgets/SCompoundWidget.h"

class FJsonObject;
class SVerticalBox;
typedef TSharedPtr<class ILaunchPadDataSource> ILaunchPadDataSourcePtr;

/////////////// Page /////////////// 
class SLaunchPadPage : public SCompoundWidget {
public:
    virtual FString GetTitle() = 0;
    FLaunchPadPageLinkClicked& GetOnLinkClicked() { return OnLinkClicked; }

protected:
    FLaunchPadPageLinkClicked OnLinkClicked;
};


class FLaunchPadPageWidgetFactory {
public:
    static TSharedPtr<SLaunchPadPage> Create(ILaunchPadDataSourcePtr InDataSource, const FString& InPath);
};

