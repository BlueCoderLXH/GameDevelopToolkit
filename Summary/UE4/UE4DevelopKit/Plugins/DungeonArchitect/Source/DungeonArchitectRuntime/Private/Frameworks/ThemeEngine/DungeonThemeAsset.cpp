//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"


UDungeonThemeAsset::UDungeonThemeAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    PreviewViewportProperties = ObjectInitializer.CreateDefaultSubobject<UDungeonEditorViewportProperties>(
        this, "PreviewProperties");
}

void UDungeonThemeAsset::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) {
    UDungeonThemeAsset* This = CastChecked<UDungeonThemeAsset>(InThis);

#if WITH_EDITORONLY_DATA
    Collector.AddReferencedObject(This->UpdateGraph, This);
#endif	// WITH_EDITORONLY_DATA

    Super::AddReferencedObjects(This, Collector);
}

