//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"

#include "Frameworks/Snap/Lib/Connection/SnapConnectionComponent.h"
#include "Frameworks/Snap/Lib/Theming/SnapTheme.h"
#include "Frameworks/ThemeEngine/DungeonThemeEngine.h"

#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/DecalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnapConnectionActor, Log, All);

ASnapConnectionActor::ASnapConnectionActor(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
{
    ConnectionId = FGuid::NewGuid();
    ConnectionComponent = ObjectInitializer.CreateDefaultSubobject<USnapConnectionComponent>(this, "Connection");
    RootComponent = ConnectionComponent;
#if WITH_EDITORONLY_DATA
    ArrowComponent = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UArrowComponent>(this, TEXT("Arrow"));
    DoorSpriteComponent = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("DoorSprite"));
    ConnectionConstraintSpriteComponent = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("ConstraintSprite"));
    //ConstraintDebugDecal = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UDecalComponent>(this, TEXT("ConstraintDebugDecal"));

    if (!IsRunningCommandlet()) {
        const TCHAR* PathName = TEXT("/DungeonArchitect/Core/Textures/S_SnapDoor");
        // Structure to hold one-time initialization
        struct FConstructorStatics {
            ConstructorHelpers::FObjectFinderOptional<UTexture2D> IconDoor;
            ConstructorHelpers::FObjectFinderOptional<UTexture2D> IconMagnet;
            ConstructorHelpers::FObjectFinderOptional<UTexture2D> IconPlugMale;
            ConstructorHelpers::FObjectFinderOptional<UTexture2D> IconPlugFemale;
            ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> MaterialDebugDecal;

            FConstructorStatics()
                : IconDoor(TEXT("/DungeonArchitect/Core/Textures/S_SnapDoor"))
                  , IconMagnet(TEXT("/DungeonArchitect/Core/Textures/icon_Magnet_256x"))
                  , IconPlugMale(TEXT("/DungeonArchitect/Core/Textures/icon_PlugMale_256x"))
                  , IconPlugFemale(TEXT("/DungeonArchitect/Core/Textures/icon_PlugFemale_256x"))
                  , MaterialDebugDecal(TEXT("/DungeonArchitect/Core/Materials/Snap/Decals/M_SnapDecalBase")) {
            }
        };
        static FConstructorStatics ConstructorStatics;

        if (ArrowComponent) {
            ArrowComponent->SetMobility(EComponentMobility::Movable);
            ArrowComponent->SetupAttachment(RootComponent);
            ArrowComponent->SetHiddenInGame(true);
        }

        if (DoorSpriteComponent) {
            DoorSpriteComponent->SetMobility(EComponentMobility::Movable);
            DoorSpriteComponent->SetupAttachment(RootComponent);
            DoorSpriteComponent->Sprite = ConstructorStatics.IconDoor.Get();
            DoorSpriteComponent->SetRelativeLocation(FVector(0, 0, 40));
            DoorSpriteComponent->SetRelativeScale3D(FVector(1, 1, 1));
        }

        if (ConnectionConstraintSpriteComponent) {
            ConnectionConstraintSpriteComponent->SetMobility(EComponentMobility::Movable);
            ConnectionConstraintSpriteComponent->SetupAttachment(RootComponent);
        }
        /*
        if (ConstraintDebugDecal) {
            ConstraintDebugDecal->SetMobility(EComponentMobility::Movable);
            ConstraintDebugDecal->SetupAttachment(RootComponent);
            ConstraintDebugDecal->SetRelativeLocation(FVector(-200, 0, 0));
            ConstraintDebugDecal->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 90, 180)));
            ConstraintDebugDecal->DecalSize = FVector(100, 100, 100);
        }
        */

        if (ConnectionConstraintSpriteComponent) {
            ConnectionConstraintSpriteComponent->SetRelativeLocation(FVector(0, 0, 100));
            ConnectionConstraintSpriteComponent->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.45f));
        }

        IconConstraintMagnet = ConstructorStatics.IconMagnet.Get();
        IconConstraintPlugMale = ConstructorStatics.IconPlugMale.Get();
        IconConstraintPlugFemale = ConstructorStatics.IconPlugFemale.Get();
        MaterialDebugDecal = ConstructorStatics.MaterialDebugDecal.Get();

        if (MaterialDebugDecal) {
            MaterialDebugDecalInstance = UMaterialInstanceDynamic::Create(MaterialDebugDecal, GetTransientPackage());
        }
    }

#endif //WITH_EDITORONLY_DATA
}

void ASnapConnectionActor::PostLoad() {
    Super::PostLoad();

    Initialize();
}


void ASnapConnectionActor::PostActorCreated() {
    Super::PostActorCreated();

    Initialize();
}

void ASnapConnectionActor::PostDuplicate(EDuplicateMode::Type DuplicateMode) {
    Super::PostDuplicate(DuplicateMode);

    if (DuplicateMode == EDuplicateMode::Normal) {
        ConnectionId = FGuid::NewGuid();
    }
}

void ASnapConnectionActor::Initialize() {
    //BuildConnection(GetWorld());

#if WITH_EDITORONLY_DATA
    UpdateConstraintIcon();
#endif	// WITH_EDITORONLY_DATA
}

#if WITH_EDITOR

void ASnapConnectionActor::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
    FName PropertyName = (e.Property != nullptr) ? e.Property->GetFName() : NAME_None;
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ASnapConnectionActor, ConnectionComponent)) {
        //BuildConnection(GetWorld());
    }
    Super::PostEditChangeProperty(e);
}

#endif


#if WITH_EDITORONLY_DATA

UTexture2D* ASnapConnectionActor::GetConstraintTexture(ESnapConnectionConstraint ConnectionConstraint) {
    switch (ConnectionComponent->ConnectionConstraint) {
    case ESnapConnectionConstraint::Magnet:
        return IconConstraintMagnet;

    case ESnapConnectionConstraint::PlugMale:
        return IconConstraintPlugMale;

    case ESnapConnectionConstraint::PlugFemale:
        return IconConstraintPlugFemale;

    default:
        return nullptr;
    }
}


void ASnapConnectionActor::UpdateConstraintIcon() {
    if (!ConnectionConstraintSpriteComponent) return;

    UTexture2D* Sprite = ConnectionComponent
                             ? GetConstraintTexture(ConnectionComponent->ConnectionConstraint)
                             : nullptr;
    ConnectionConstraintSpriteComponent->SetSprite(Sprite);

    UpdateConstraintDecal();
}

void ASnapConnectionActor::UpdateConstraintDecal() {
    if (!ConstraintDebugDecal) return;

    const bool bIsDoor = (ConnectionComponent->ConnectionState == ESnapConnectionState::Door);
    if (bIsDoor && MaterialDebugDecalInstance) {
        UTexture2D* Sprite = ConnectionComponent
                                 ? GetConstraintTexture(ConnectionComponent->ConnectionConstraint)
                                 : nullptr;
        MaterialDebugDecalInstance->SetTextureParameterValue("Texture", Sprite);
        ConstraintDebugDecal->SetDecalMaterial(MaterialDebugDecalInstance);
        ConstraintDebugDecal->SetVisibility(true);
    }
    else {
        ConstraintDebugDecal->SetDecalMaterial(nullptr);
        ConstraintDebugDecal->SetVisibility(false);
    }
}

#endif	// WITH_EDITORONLY_DATA

void ASnapConnectionActor::BuildImpl(int32 InSeed, ULevel* HostLevel, const int32 ConnectionOrderNum) {
    USnapConnectionInfo* ConnectionInfo = ConnectionComponent->ConnectionInfo;
    if (!ConnectionInfo->ThemeAsset) {
        UE_LOG(LogSnapConnectionActor, Error, TEXT("Missing Theme asset reference. Cannot build snap connection instance"));
        return;
    }
    
    RootComponent->SetMobility(EComponentMobility::Static);
    const ESnapConnectionState ConnectionState = ConnectionComponent->ConnectionState;

    FString MarkerName;
    switch(ConnectionState) {
        case ESnapConnectionState::Wall:
            MarkerName = FSnapConnectionMarkers::Wall;
            break;

        case ESnapConnectionState::Door:
            {
                switch(ConnectionComponent->DoorType) {
                case ESnapConnectionDoorType::NormalDoor:
                    MarkerName = FSnapConnectionMarkers::Door;
                    break;
                case ESnapConnectionDoorType::OneWayDoor:
                    MarkerName = FSnapConnectionMarkers::OneWayDoor;
                    break;
                case ESnapConnectionDoorType::OneWayDoorUp:
                    MarkerName = FSnapConnectionMarkers::OneWayDoorUp;
                    break;
                case ESnapConnectionDoorType::OneWayDoorDown:
                    MarkerName = FSnapConnectionMarkers::OneWayDoorDown;
                    break;
                case ESnapConnectionDoorType::LockedDoor:
                case ESnapConnectionDoorType::CustomDoor:
                default:
                    MarkerName = ConnectionComponent->MarkerName;
                    break;
                }
                break;
            }
        default:
            // Unsupported marker
            return;
    }

    TArray<FPropSocket> MarkersToEmit;
    FPropSocket& StartMarker = MarkersToEmit.AddDefaulted_GetRef();
    StartMarker.Id = 0;
    StartMarker.SocketType = MarkerName;
    StartMarker.Transform = GetActorTransform(); 

    UWorld* World = GetWorld();
    ULevel* LevelOverride = (ConnectionState == ESnapConnectionState::Door) ? World->PersistentLevel : GetLevel();
    TSharedPtr<FSnapThemeSceneProvider> SceneProvider = MakeShareable(new FSnapThemeSceneProvider(World));
    SceneProvider->SetLevelOverride(LevelOverride);
    FDungeonThemeEngineSettings ThemeEngineSettings;
    ThemeEngineSettings.Themes = { ConnectionInfo->ThemeAsset };
    ThemeEngineSettings.SceneProvider = SceneProvider;

    const FRandomStream Random(InSeed);
    const FDungeonThemeEngineEventHandlers ThemeEventHandlers;
    
    // Invoke the Theme Engine
    FDungeonThemeEngine::Apply(MarkersToEmit, Random, ThemeEngineSettings, ThemeEventHandlers);

    SpawnedInstances = SceneProvider->GetSpawnedActors();

    if (!ConnectionComponent->SpawnOffset.Equals(FTransform::Identity)) {
        for (TWeakObjectPtr<AActor> SpawnedActor : SpawnedInstances) {
            FTransform BaseTransform = SpawnedActor->GetActorTransform();
            FTransform FinalTransform = ConnectionComponent->SpawnOffset * BaseTransform;
            if (SpawnedActor->GetRootComponent()) {
                EComponentMobility::Type OriginalMobility = SpawnedActor->GetRootComponent()->Mobility;
                if (OriginalMobility != EComponentMobility::Movable) {
                    SpawnedActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
                }
                SpawnedActor->SetActorTransform(FinalTransform);
                if (OriginalMobility != EComponentMobility::Movable) {
                    SpawnedActor->GetRootComponent()->SetMobility(OriginalMobility);
                }
            }
        }
    }

    // Attach the spawned instances to this actor
    const FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, false);
    for (TWeakObjectPtr<AActor> SpawnedInstance : SpawnedInstances) {
        if (SpawnedInstance.IsValid()) {
            SpawnedInstance->AttachToActor(this, AttachmentRules);

            // 序号大于0才对门编号
            if (ConnectionOrderNum > 0)
            {
                SpawnedInstance->Tags.Add(FName(FString::FormatAsNumber(ConnectionOrderNum)));
            }
        }
    }
}

void ASnapConnectionActor::BuildImplDeprecated(ULevel* HostLevel, const int32 ConnectionOrderNum) {
    USnapConnectionInfo* ConnectionInfo = ConnectionComponent->ConnectionInfo;
    const ESnapConnectionState ConnectionState = ConnectionComponent->ConnectionState;
    
    if (ConnectionInfo->Version == static_cast<int32>(ESnapConnectionInfoVersion::InitialVersion)) {
        switch (ConnectionState) {
        case ESnapConnectionState::Door:
            CreateInstanceDeprecated(ConnectionInfo->DoorVisuals, HostLevel);
            break;

        case ESnapConnectionState::Wall:
            CreateInstanceDeprecated(ConnectionInfo->WallVisuals, HostLevel);
            break;

        case ESnapConnectionState::Unknown:
        default:
            UE_LOG(LogSnapConnection, Warning, TEXT("SnapConnectionActor: Unsupported door type"));
            break;
        }
    }
}


void ASnapConnectionActor::CreateInstanceDeprecated(const FSnapConnectionVisualInfo_DEPRECATED& VisualInfo, ULevel* HostLevel) {
    UWorld* World = GetWorld();
    if (!World) {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.ObjectFlags = RF_Transient;
    SpawnParams.OverrideLevel = HostLevel;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* InstanceActor = nullptr;
    if (VisualInfo.bStaticMesh) {
        AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(SpawnParams);
        if (MeshActor) {
            UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
            MeshComponent->SetMobility(EComponentMobility::Movable);
            MeshActor->SetActorRelativeTransform(VisualInfo.MeshInfo.Offset);
            MeshComponent->SetStaticMesh(VisualInfo.MeshInfo.StaticMesh);
            if (VisualInfo.MeshInfo.MaterialOverride) {
                MeshComponent->SetMaterial(0, VisualInfo.MeshInfo.MaterialOverride);
            }

            InstanceActor = MeshActor;
        }
    }
    else {
        AActor* BPActor = World->SpawnActor<AActor>(VisualInfo.BlueprintInfo.BlueprintClass, SpawnParams);
        if (BPActor) {
            if (!World->IsGameWorld()) {
                BPActor->OnConstruction(FTransform::Identity);
            }
            BPActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
            BPActor->SetActorRelativeTransform(VisualInfo.BlueprintInfo.Offset);

            InstanceActor = BPActor;
        }
    }

    if (!InstanceActor) {
        return;
    }

    const FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, false);
    InstanceActor->AttachToActor(this, AttachmentRules);

    SpawnedInstances.Add(InstanceActor);
    
#if WITH_EDITORONLY_DATA
    UpdateConstraintDecal();
#endif // WITH_EDITORONLY_DATA
}

TArray<AActor*> ASnapConnectionActor::GetSpawnedInstances() const {
    TArray<AActor*> Result;
    for (const TWeakObjectPtr<AActor> SpawnedInstance : SpawnedInstances) {
        if (SpawnedInstance.IsValid()) {
            Result.Add(SpawnedInstance.Get());
        }
    }
    return Result;
}

void ASnapConnectionActor::BuildConnectionInstance(ULevel* InHostLevel, const int32 ConnectionOrderNum) {
    if (!ConnectionComponent) {
        return;
    }

    USnapConnectionInfo* ConnectionInfo = ConnectionComponent->ConnectionInfo;

    if (!ConnectionInfo) {
        return;
    }

    DestroyConnectionInstance();
    check(SpawnedInstances.Num() == 0);

    if (ConnectionInfo->Version == static_cast<int32>(ESnapConnectionInfoVersion::LatestVersion)) {
        const int32 Seed = GetTypeHash(GetActorLocation()); 
        BuildImpl(Seed, InHostLevel, ConnectionOrderNum);
    }
    else {
        BuildImplDeprecated(InHostLevel, ConnectionOrderNum);
    }
}

void ASnapConnectionActor::DestroyConnectionInstance() {
    for (const TWeakObjectPtr<AActor> SpawnedInstance : SpawnedInstances) {
        if (SpawnedInstance.IsValid()) {
            SpawnedInstance->Destroy();
        }
    }
    SpawnedInstances.Reset();
}

void ASnapConnectionActor::SetHiddenInGame(bool bInHidden) {
    for (TWeakObjectPtr<AActor> SpawnedInstance : SpawnedInstances) {
        if (SpawnedInstance.IsValid()) {
            SpawnedInstance->SetActorHiddenInGame(bInHidden);
        }
    }
}

