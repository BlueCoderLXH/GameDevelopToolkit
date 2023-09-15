//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

DECLARE_DELEGATE_OneParam(FLaunchPadPageNavigation, TSharedPtr<class SLaunchPadPage>);
DECLARE_DELEGATE_OneParam(FLaunchPadPageLinkClicked, const FString&);

