//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/DungeonNavLinkProxy.h"

#include "NavLinkCustomComponent.h"

void ADungeonNavLinkProxy::SetupSmartLinkData(const FVector& Start, const FVector& End, ENavLinkDirection::Type Direction) {
    UNavLinkCustomComponent* SmartLinkComponent = GetSmartLinkComp();
    if (SmartLinkComponent) {
        SmartLinkComponent->SetLinkData(Start, End, Direction);
    }
}

