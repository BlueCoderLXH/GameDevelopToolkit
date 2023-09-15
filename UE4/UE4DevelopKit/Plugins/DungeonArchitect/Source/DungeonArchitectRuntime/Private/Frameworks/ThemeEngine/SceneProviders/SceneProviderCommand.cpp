//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/ThemeEngine/SceneProviders/SceneProviderCommand.h"

#include "Core/Actors/DungeonActorTemplate.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/EditorService/IDungeonEditorService.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProviderContext.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/Light.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Particles/Emitter.h"
#include "Particles/ParticleSystemComponent.h"

FName FSceneProviderCommand::TagComplexActor = "Dungeon-Complex-Actor";

void FSceneProviderCommand::Execute(UWorld* World) {
    ExecuteImpl(World);
}

void FSceneProviderCommand::PostInitializeActor(AActor* Actor) {
    ExecuteSpawnLogics(Actor, Dungeon, Context.SpawnLogics);

    if (Context.MarkerUserData.IsValid() && Actor) {
        UDungeonBuilder* DungeonBuilder = Dungeon ? Dungeon->GetBuilder() : nullptr;
        if (DungeonBuilder) {
            DungeonBuilder->ProcessThemeItemUserData(Context.MarkerUserData, Actor);
        }
    }

    OnActorSpawned.ExecuteIfBound(Actor);
}

void FSceneProviderCommand::ExecuteSpawnLogics(AActor* SpawnedActor, ADungeon* InDungeon,
                                              const TArray<UDungeonSpawnLogic*>& SpawnLogics) {
    if (!SpawnedActor) return;
    for (UDungeonSpawnLogic* SpawnLogic : SpawnLogics) {
        if (!SpawnLogic) continue;
        SpawnLogic->OnItemSpawn(SpawnedActor, InDungeon);
    }
}

void FSceneProviderCommand::AddReferencedObjects(FReferenceCollector& Collector) {
    Collector.AddReferencedObject(Dungeon);
    Collector.AddReferencedObjects(Context.SpawnLogics);
}

void FSceneProviderCommand::AddNodeTag(AActor* Actor, const FName& NodeId, bool bApplyPrefix /* = true */) {
    if (Actor) {
        FName NodeTag = bApplyPrefix ? CreateNodeTagFromId(NodeId) : NodeId;
        Actor->Tags.Add(NodeTag);

        FName DungeonIdTag = UDungeonModelHelper::GetDungeonIdTag(Dungeon);
        Actor->Tags.Add(DungeonIdTag);
    }
}

void FSceneProviderCommand::MoveToFolder(ADungeon* Dungeon, AActor* ActorToMove) {
#if WITH_EDITOR
    if (ActorToMove && Dungeon) {
        ActorToMove->SetFolderPath(Dungeon->ItemFolderPath);
    }
#endif
}

void FSceneProviderCommand::MoveToFolder(AActor* Actor) {
    MoveToFolder(Dungeon, Actor);
}

void FSceneProviderCommand::TagAsComplexObject(AActor* Actor) {
    Actor->Tags.Add(TagComplexActor);
}

void SceneProviderCommand_CreateMesh::ExecuteImpl(UWorld* World) {
    FActorSpawnParameters SpawnParams;
    SpawnParams.OverrideLevel = LevelOverride;
    SpawnParams.bDeferConstruction = true;
    AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnParams);
    UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();

    EComponentMobility::Type OriginialMobility = MeshComponent->Mobility;
    if (Mesh->Template) {
        OriginialMobility = Mesh->Template->Mobility;
    }
    MeshActor->SetMobility(EComponentMobility::Movable);
    MeshComponent->SetStaticMesh(Mesh->StaticMesh);
    MeshActor->SetMobility(OriginialMobility);
    MeshActor->FinishSpawning(Context.transform);

    for (const FMaterialOverride& MaterialOverride : Mesh->MaterialOverrides) {
        MeshComponent->SetMaterial(MaterialOverride.index, MaterialOverride.Material);
    }

    SetMeshComponentAttributes(MeshComponent, Mesh->Template);
    MeshActor->MarkComponentsRenderStateDirty();

    AddNodeTag(MeshActor, Context.NodeId);
    MoveToFolder(MeshActor);

    MeshActor->ReregisterAllComponents();

    PostInitializeActor(MeshActor);
}


#define SET_MESH_ATTRIB(Attrib) StaticMeshComponent->Attrib = StaticMeshTemplate->Attrib

void SceneProviderCommand_CreateMesh::SetMeshComponentAttributes(UStaticMeshComponent* StaticMeshComponent,
                                                                 UStaticMeshComponent* StaticMeshTemplate) {
    if (StaticMeshTemplate) {
        UEngine::CopyPropertiesForUnrelatedObjects(StaticMeshTemplate, StaticMeshComponent);

        StaticMeshComponent->SetCanEverAffectNavigation(StaticMeshTemplate->CanEverAffectNavigation());
    }
    else {
        StaticMeshComponent->SetMobility(EComponentMobility::Static);
        StaticMeshComponent->SetCanEverAffectNavigation(true);
        StaticMeshComponent->bCastStaticShadow = true;
    }
}

void SceneProviderCommand_CreateMesh::AddReferencedObjects(FReferenceCollector& Collector) {
    FSceneProviderCommand::AddReferencedObjects(Collector);
    if (Mesh) Collector.AddReferencedObject(Mesh);
}

void SceneProviderCommand_AddLight::ExecuteImpl(UWorld* World) {
    ALight* Light = nullptr;
    UPointLightComponent* PointLightComponent = nullptr;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.OverrideLevel = LevelOverride;
    SpawnParams.bDeferConstruction = true;

    if (USpotLightComponent* SpotLightTemplate = Cast<USpotLightComponent>(LightTemplate)) {
        ASpotLight* SpotLight = World->SpawnActor<ASpotLight
        >(ASpotLight::StaticClass(), Context.transform, SpawnParams);
        USpotLightComponent* SpotLightComponent = SpotLight->SpotLightComponent;
        SetSpotLightAttributes(SpotLightComponent, SpotLightTemplate);

        Light = SpotLight;
        PointLightComponent = SpotLightComponent;
    }
    else {
        APointLight* PointLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), Context.transform,
                                                                 SpawnParams);

        Light = PointLight;
        PointLightComponent = PointLight->PointLightComponent;
    }

    AddNodeTag(Light, Context.NodeId);
    MoveToFolder(Light);

    SetPointLightAttributes(PointLightComponent, LightTemplate);
    Light->FinishSpawning(Context.transform, true);
    Light->ReregisterAllComponents();

    PostInitializeActor(Light);
}

#define SET_SPOT_ATTRIB(Attrib) SpotLightComponent->Attrib = SpotLightTemplate->Attrib

void SceneProviderCommand_AddLight::SetSpotLightAttributes(USpotLightComponent* SpotLightComponent,
                                                           USpotLightComponent* SpotLightTemplate) {
    FGuid OrigLightGuid = SpotLightComponent->LightGuid;
    UEngine::CopyPropertiesForUnrelatedObjects(SpotLightTemplate, SpotLightComponent);
    SpotLightComponent->LightGuid = OrigLightGuid;
}

#define SET_LIGHT_ATTRIB(Attrib) PointLightComponent->Attrib = LightTemplate->Attrib;

void SceneProviderCommand_AddLight::SetPointLightAttributes(UPointLightComponent* PointLightComponent,
                                                            UPointLightComponent* LightTemplate) {
    FGuid OrigLightGuid = PointLightComponent->LightGuid;
    UEngine::CopyPropertiesForUnrelatedObjects(LightTemplate, PointLightComponent);
    PointLightComponent->LightGuid = OrigLightGuid;
}


void SceneProviderCommand_AddLight::AddReferencedObjects(FReferenceCollector& Collector) {
    FSceneProviderCommand::AddReferencedObjects(Collector);
    if (LightTemplate) Collector.AddReferencedObject(LightTemplate);
}

void SceneProviderCommand_AddParticleSystem::AddReferencedObjects(FReferenceCollector& Collector) {
    FSceneProviderCommand::AddReferencedObjects(Collector);
    if (ParticleTemplate) Collector.AddReferencedObject(ParticleTemplate);
}

void SceneProviderCommand_AddParticleSystem::ExecuteImpl(UWorld* World) {
    AEmitter* ParticleEmitter = AddActor<AEmitter>(World, LevelOverride, Context.transform, Context.NodeId);
    ParticleEmitter->GetParticleSystemComponent()->SetTemplate(ParticleTemplate);

    PostInitializeActor(ParticleEmitter);
}

void SetActorTransform(AActor* Actor, const FTransform& transform) {
    if (!Actor) return;
    USceneComponent* SceneComponent = Actor->GetRootComponent();

    if (SceneComponent) {
        EComponentMobility::Type OriginialMobility = SceneComponent->Mobility;
        SceneComponent->SetMobility(EComponentMobility::Movable);
        SceneComponent->SetWorldTransform(transform);
        SceneComponent->UpdateChildTransforms();
        SceneComponent->SetMobility(OriginialMobility);
        SceneComponent->ReregisterComponent();
    }

    //Actor->RerunConstructionScripts();
}

void SceneProviderCommand_AddActor::AddReferencedObjects(FReferenceCollector& Collector) {
    FSceneProviderCommand::AddReferencedObjects(Collector);
    if (ClassTemplate) Collector.AddReferencedObject(ClassTemplate);
}

void SceneProviderCommand_AddActor::ExecuteImpl(UWorld* World) {
    FActorSpawnParameters SpawnParams;
    SpawnParams.OverrideLevel = LevelOverride;

    AActor* Actor = World->SpawnActor<AActor>(ClassTemplate, Context.transform, SpawnParams);
    if (Actor) {
        if (World->WorldType == EWorldType::Editor) {
            Actor->RerunConstructionScripts();
        }
        AddNodeTag(Actor, Context.NodeId);
        MoveToFolder(Actor);

        TagAsComplexObject(Actor);

        PostInitializeActor(Actor);
    }
}

void SceneProviderCommand_CloneActor::AddReferencedObjects(FReferenceCollector& Collector) {
    FSceneProviderCommand::AddReferencedObjects(Collector);
    if (ActorTemplate) Collector.AddReferencedObject(ActorTemplate);
}

void SceneProviderCommand_CloneActor::ExecuteImpl(UWorld* World) {
    if (!ActorTemplate) return;
    if (!ActorTemplate->ClassTemplate) {
        ActorTemplate->ActorTemplate = nullptr;
        return;
    }

    AActor* TargetActorTemplate = ActorTemplate->ActorTemplate;

    // Check if the template actor class matches the latest version
    if (!TargetActorTemplate || TargetActorTemplate->GetClass() != ActorTemplate->ClassTemplate) {
        // The actor version does not match with the class version (the class was modified after the actor template was created)
        // Create a new template and copy the parameters over
        TargetActorTemplate = NewObject<AActor>(World, ActorTemplate->ClassTemplate, NAME_None,
                                                RF_ArchetypeObject | RF_Transactional | RF_Public);
        if (ActorTemplate->ActorTemplate) {
            UEngine::CopyPropertiesForUnrelatedObjects(ActorTemplate->ActorTemplate, TargetActorTemplate);
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.OverrideLevel = LevelOverride;
    SpawnParams.Template = TargetActorTemplate;
    SpawnParams.bDeferConstruction = true;
    AActor* Actor = World->SpawnActor<AActor>(ActorTemplate->ClassTemplate, SpawnParams);
    if (Actor) {
        Actor->FinishSpawning(Context.transform);

        if (World->WorldType == EWorldType::Editor) {
            Actor->RerunConstructionScripts();
        }

        AddNodeTag(Actor, Context.NodeId);
        MoveToFolder(Actor);


        TagAsComplexObject(Actor);

        PostInitializeActor(Actor);
    }
}

void SceneProviderCommand_SetActorTransform::AddReferencedObjects(FReferenceCollector& Collector) {
    FSceneProviderCommand::AddReferencedObjects(Collector);
    if (Actor) Collector.AddReferencedObject(Actor);
}

void SceneProviderCommand_SetActorTransform::ExecuteImpl(UWorld* World) {
    SetActorTransform(Actor, Context.transform);
}

void SceneProviderCommand_CreateGroupActor::ExecuteImpl(UWorld* World) {
    TSharedPtr<IDungeonEditorService> EditorService = IDungeonEditorService::Get();
    if (!EditorService.IsValid()) {
        // Editor service not specified. Cannot create editor specific functionality (e.g. when running as standalone game)
        return;
    }

    TArray<FName> GroupActorNodeTags;
    for (const FName& ActorNodeId : ActorNodeIds) {
        GroupActorNodeTags.Add(CreateNodeTagFromId(ActorNodeId));
    }

    TArray<AActor*> GroupedActors;
    for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt) {
        AActor* Actor = *ActorIt;
        for (const FName& Tag : Actor->Tags) {
            if (GroupActorNodeTags.Contains(Tag)) {
                GroupedActors.Add(Actor);
                break;
            }
        }
    }

    AActor* GroupActor = EditorService->CreateGroupActor(World, GroupedActors, Context.transform);
    if (GroupActor) {
        AddNodeTag(GroupActor, Context.NodeId);
        MoveToFolder(GroupActor);
    }
}

void SceneProviderCommand_DestroyActorWithTag::ExecuteImpl(UWorld* World) {
    FName NodeTag = CreateNodeTagFromId(Tag);
    for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt) {
        AActor* Actor = *ActorIt;
        if (Actor->Tags.Contains(NodeTag)) {
            Actor->Destroy();
            break;
        }
    }
}

void SceneProviderCommand_ReuseActor::AddReferencedObjects(FReferenceCollector& Collector) {
    FSceneProviderCommand::AddReferencedObjects(Collector);
    if (ActorToReuse) Collector.AddReferencedObject(ActorToReuse);
}

void SceneProviderCommand_ReuseActor::ExecuteImpl(UWorld* World) {
    if (ActorToReuse) {
        if (bRerunConstructionScripts && World && World->WorldType == EWorldType::Editor) {
            ActorToReuse->RerunConstructionScripts();
        }
        if (!ActorToReuse->GetTransform().Equals(Context.transform)) {
            SetActorTransform(ActorToReuse, Context.transform);
        }

        PostInitializeActor(ActorToReuse);
    }
}

void SceneProviderCommand_ReuseStaticMesh::AddReferencedObjects(FReferenceCollector& Collector) {
    SceneProviderCommand_ReuseActor::AddReferencedObjects(Collector);
    if (Mesh) Collector.AddReferencedObject(Mesh);
}

void SceneProviderCommand_ReuseStaticMesh::ExecuteImpl(UWorld* World) {
    if (Mesh) {
        if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(ActorToReuse)) {
            UStaticMeshComponent* MeshComponent = StaticMeshActor->GetStaticMeshComponent();
            MeshComponent->SetStaticMesh(Mesh->StaticMesh);

            // Set the mesh component attributes
            SceneProviderCommand_CreateMesh::SetMeshComponentAttributes(MeshComponent, Mesh->Template);
            StaticMeshActor->MarkComponentsRenderStateDirty();
        }
    }

    // Call the base class
    bRerunConstructionScripts = false;
    SceneProviderCommand_ReuseActor::ExecuteImpl(World);
}

void SceneProviderCommand_ReuseLight::AddReferencedObjects(FReferenceCollector& Collector) {
    SceneProviderCommand_ReuseActor::AddReferencedObjects(Collector);
    if (LightTemplate) Collector.AddReferencedObject(LightTemplate);
}

void SceneProviderCommand_ReuseLight::ExecuteImpl(UWorld* World) {
    if (USpotLightComponent* SpotLightTemplate = Cast<USpotLightComponent>(LightTemplate)) {
        if (ASpotLight* SpotLightActor = Cast<ASpotLight>(ActorToReuse)) {
            SceneProviderCommand_AddLight::SetSpotLightAttributes(SpotLightActor->SpotLightComponent,
                                                                  SpotLightTemplate);
            SceneProviderCommand_AddLight::SetPointLightAttributes(SpotLightActor->SpotLightComponent,
                                                                   SpotLightTemplate);
        }
    }
    else if (APointLight* PointLight = Cast<APointLight>(ActorToReuse)) {
        SceneProviderCommand_AddLight::SetPointLightAttributes(PointLight->PointLightComponent, LightTemplate);
    }

    // Call the base class
    SceneProviderCommand_ReuseActor::ExecuteImpl(World);
}

void SceneProviderCommand_ReuseParticleSystem::AddReferencedObjects(FReferenceCollector& Collector) {
    SceneProviderCommand_ReuseActor::AddReferencedObjects(Collector);
    if (ParticleTemplate) Collector.AddReferencedObject(ParticleTemplate);
}

void SceneProviderCommand_ReuseParticleSystem::ExecuteImpl(UWorld* World) {
    AEmitter* EmitterActor = Cast<AEmitter>(ActorToReuse);
    if (EmitterActor) {
        EmitterActor->SetTemplate(ParticleTemplate);
    }

    // Call the base class
    SceneProviderCommand_ReuseActor::ExecuteImpl(World);
}

void SceneProviderCommand_ReuseActorTemplate::AddReferencedObjects(FReferenceCollector& Collector) {
    SceneProviderCommand_ReuseActor::AddReferencedObjects(Collector);
    if (ClassTemplate) Collector.AddReferencedObject(ClassTemplate);
}

void SceneProviderCommand_ReuseActorTemplate::ExecuteImpl(UWorld* World) {
    if (ClassTemplate && ActorToReuse && !ActorToReuse->IsA(ClassTemplate)) {
        ActorToReuse->Destroy();
        ActorToReuse = nullptr;

        SceneProviderCommand_AddActor AddActorCommand(Dungeon, LevelOverride, Context, ClassTemplate);
        AddActorCommand.Execute(World);
    }
    else {
        // The desired template class is correct. call the base class
        SceneProviderCommand_ReuseActor::ExecuteImpl(World);
    }
}

void SceneProviderCommand_ReuseClonedActor::AddReferencedObjects(FReferenceCollector& Collector) {
    SceneProviderCommand_ReuseActor::AddReferencedObjects(Collector);
    if (ActorTemplate) Collector.AddReferencedObject(ActorTemplate);
}

void SceneProviderCommand_ReuseClonedActor::ExecuteImpl(UWorld* World) {
    if (ActorTemplate && !ActorToReuse->IsA(ActorTemplate->GetClass())) {
        ActorToReuse->Destroy();
        ActorToReuse = nullptr;

        SceneProviderCommand_CloneActor CloneActorCommand(Dungeon, LevelOverride, Context, ActorTemplate);
        CloneActorCommand.Execute(World);
    }
    else {
        // The desired template class is correct. call the base class
        SceneProviderCommand_ReuseActor::ExecuteImpl(World);
    }
}

void SceneProviderCommand_DestroyActor::AddReferencedObjects(FReferenceCollector& Collector) {
    FSceneProviderCommand::AddReferencedObjects(Collector);
    if (Actor) Collector.AddReferencedObject(Actor);
}

