//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionComponent.h"

#include "GameFramework/Actor.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "SnapConnectionActor.generated.h"

class AActor;
class UArrowComponent;
class UDecalComponent;
class UBillboardComponent;
class UDecalComponent;
class UTexture2D;
class UMaterialInterface;
class UMaterialInstanceDynamic;


UCLASS(Blueprintable, ConversionRoot, ComponentWrapperClass)
class DUNGEONARCHITECTRUNTIME_API ASnapConnectionActor : public AActor {
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY()
    USnapConnectionComponent* ConnectionComponent;

private:
    UPROPERTY()
    FGuid ConnectionId;

public:
    virtual void PostLoad() override;
    virtual void PostActorCreated() override;
    virtual bool IsLevelBoundsRelevant() const override { return false; }
    virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;

    FGuid GetConnectionId() const { return ConnectionId; }

#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif

#if WITH_EDITORONLY_DATA
    UTexture2D* GetConstraintTexture(ESnapConnectionConstraint ConnectionConstraint);
    void UpdateConstraintIcon();
    void UpdateConstraintDecal();
#endif	// WITH_EDITORONLY_DATA


public:
    void BuildConnectionInstance(ULevel* InHostLevel = nullptr, const int32 ConnectionOrderNum = -1);
    void DestroyConnectionInstance();
    void SetHiddenInGame(bool bInHidden);
    TArray<TWeakObjectPtr<AActor>> GetSpawnedInstancesPtr() const { return SpawnedInstances; }
    
private:
    void Initialize();
    void BuildImpl(int32 InSeed, ULevel* HostLevel = nullptr, const int32 ConnectionOrderNum = -1);
    void BuildImplDeprecated(ULevel* HostLevel = nullptr, const int32 ConnectionOrderNum = -1);
    void CreateInstanceDeprecated(const FSnapConnectionVisualInfo_DEPRECATED& VisualInfo, ULevel* HostLevel = nullptr);

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dungeon")
    TArray<AActor*> GetSpawnedInstances() const;
    
private:
    TArray<TWeakObjectPtr<AActor>> SpawnedInstances;

#if WITH_EDITORONLY_DATA
    UPROPERTY()
    UBillboardComponent* DoorSpriteComponent;

    UPROPERTY()
    UBillboardComponent* ConnectionConstraintSpriteComponent;

    UPROPERTY()
    UDecalComponent* ConstraintDebugDecal;

    UPROPERTY(Transient)
    UArrowComponent* ArrowComponent;

    UPROPERTY(Transient)
    UTexture2D* IconConstraintMagnet;

    UPROPERTY(Transient)
    UTexture2D* IconConstraintPlugMale;

    UPROPERTY(Transient)
    UTexture2D* IconConstraintPlugFemale;

    UPROPERTY(Transient)
    UMaterialInterface* MaterialDebugDecal;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* MaterialDebugDecalInstance;


#endif //WITH_EDITORONLY_DATA
};

