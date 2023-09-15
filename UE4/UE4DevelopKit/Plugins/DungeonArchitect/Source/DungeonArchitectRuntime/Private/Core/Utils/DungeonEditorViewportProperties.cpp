//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/DungeonEditorViewportProperties.h"

#include "Core/DungeonBuilder.h"

UDungeonEditorViewportProperties::UDungeonEditorViewportProperties(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    DungeonConfig = ObjectInitializer.CreateDefaultSubobject<UDungeonConfig>(this, "Config");
    BuilderClass = UDungeonBuilder::DefaultBuilderClass();
}

#if WITH_EDITOR
void UDungeonEditorViewportProperties::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
    Super::PostEditChangeProperty(e);

    if (PropertyChangeListener.IsValid()) {
        PropertyChangeListener.Pin()->OnPropertyChanged(e.Property->GetName(), this);
    }
}

void UDungeonEditorViewportProperties::PostEditChangeConfigProperty(struct FPropertyChangedEvent& e) {
    if (PropertyChangeListener.IsValid()) {
        PropertyChangeListener.Pin()->OnPropertyChanged(e.Property->GetName(), this);
    }
}
#endif // WITH_EDITOR

