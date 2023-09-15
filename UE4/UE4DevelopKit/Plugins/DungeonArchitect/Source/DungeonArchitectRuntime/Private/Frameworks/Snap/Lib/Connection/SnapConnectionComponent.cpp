//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Snap/Lib/Connection/SnapConnectionComponent.h"

#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"

DEFINE_LOG_CATEGORY(LogSnapConnection);

USnapConnectionComponent::USnapConnectionComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void USnapConnectionComponent::OnRegister() {
    Super::OnRegister();

    CreationMethod = EComponentCreationMethod::Instance;
}


void USnapConnectionComponent::Serialize(FArchive& Ar) {
    Super::Serialize(Ar);

    Ar << ConnectionState;
}


#if WITH_EDITOR
void USnapConnectionComponent::PostEditChangeProperty(FPropertyChangedEvent& e) {
    FName PropertyName = (e.Property != nullptr) ? e.Property->GetFName() : NAME_None;
    bool bRebuildConnection = false;
    if (PropertyName == GET_MEMBER_NAME_CHECKED(USnapConnectionComponent, ConnectionInfo)) {
        bRebuildConnection = true;
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(USnapConnectionComponent, ConnectionState)) {
        bRebuildConnection = true;
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(USnapConnectionComponent, ConnectionConstraint)) {
        ASnapConnectionActor* ConnectionActor = Cast<ASnapConnectionActor>(GetOwner());
        if (ConnectionActor) {
            ConnectionActor->UpdateConstraintIcon();
        }
    }

    if (bRebuildConnection) {
        ASnapConnectionActor* ConnectionActor = Cast<ASnapConnectionActor>(GetOwner());
        if (ConnectionActor) {
            //ConnectionActor->BuildConnection(GetWorld());
        }
    }


    Super::PostEditChangeProperty(e);
}
#endif

