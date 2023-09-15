//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonActorTemplate.h"

#include "Core/Actors/DungeonActorTemplate.h"

#include "ActorFactories/ActorFactory.h"
#include "AssetRegistry/AssetData.h"
#include "ComponentAssetBroker.h"

void UEdGraphNode_DungeonActorTemplate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UEdGraphNode_DungeonActorTemplate::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) {
    if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(
        UEdGraphNode_DungeonActorTemplate, ClassTemplate)) {
        SetTemplateClass(ClassTemplate);
    }
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UEdGraphNode_DungeonActorTemplate::OnThemeEditorLoaded() {
    if (!ActorTemplate) {
        SetTemplateClass(ClassTemplate);
    }
}

void UEdGraphNode_DungeonActorTemplate::SetTemplateClass(TSubclassOf<AActor> InClass) {
    ClassTemplate = InClass;

    if (ClassTemplate) {
        if (ActorTemplate == nullptr || ActorTemplate->GetClass() != ClassTemplate) {
            Modify();

            AActor* NewActorTemplate = NewObject<AActor>(GetTransientPackage(), ClassTemplate, NAME_None,
                                                         RF_ArchetypeObject | RF_Transactional | RF_Public);

            if (ActorTemplate) {
                UEngine::CopyPropertiesForUnrelatedObjects(ActorTemplate, NewActorTemplate);

                ActorTemplate->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
            }

            ActorTemplate = NewActorTemplate;

            // Record initial object state in case we're in a transaction context.
            ActorTemplate->Modify();

            // Now set the actual name and outer to the BPGC.
            const FString TemplateName = FString::Printf(TEXT("%s_%s_CAT"), *GetName(), *ClassTemplate->GetName());

            ActorTemplate->Rename(*TemplateName, this,
                                  REN_DoNotDirty | REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
        }
    }
    else {
        if (ActorTemplate) {
            Modify();
            ActorTemplate->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
            ActorTemplate = nullptr;
        }
    }
}

void UEdGraphNode_DungeonActorTemplate::SetTemplateFromAsset(UObject* AssetObject, UActorFactory* ActorFactory) {
    if (AssetObject && ActorFactory) {
        FAssetData AssetData(AssetObject);
        SetTemplateClass(ActorFactory->GetDefaultActorClass(AssetData));
        if (ActorTemplate) {
            FComponentAssetBrokerage::AssignAssetToComponent(ActorTemplate->GetRootComponent(), AssetObject);
        }
    }
    else {
        ActorTemplate = nullptr;
        ClassTemplate = nullptr;
    }
}


UObject* UEdGraphNode_DungeonActorTemplate::GetNodeAssetObject(UObject* Outer) {
    UDungeonActorTemplate* TemplateObject = NewObject<UDungeonActorTemplate>(Outer);

    AActor* NewActorTemplate = nullptr;
    if (ClassTemplate) {
        // Clone the actor template 
        NewActorTemplate = NewObject<AActor>(TemplateObject, ClassTemplate, NAME_None,
                                             RF_ArchetypeObject | RF_Transactional | RF_Public);

        if (ActorTemplate) {
            UEngine::CopyPropertiesForUnrelatedObjects(ActorTemplate, NewActorTemplate);
        }
    }

    TemplateObject->ActorTemplate = NewActorTemplate;
    TemplateObject->ClassTemplate = ClassTemplate;
    return TemplateObject;
}

UObject* UEdGraphNode_DungeonActorTemplate::GetThumbnailAssetObject() {
    if (ActorTemplate) {
        UActorComponent* RootComponent = ActorTemplate->GetRootComponent();
        if (RootComponent) {
            UObject* AssetObject = FComponentAssetBrokerage::GetAssetFromComponent(RootComponent);
            if (AssetObject) {
                return AssetObject;
            }
        }
        return ActorTemplate->GetClass();
    }
    return ClassTemplate;
}

