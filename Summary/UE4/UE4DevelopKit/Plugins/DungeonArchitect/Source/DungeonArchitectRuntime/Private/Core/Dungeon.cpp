//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Dungeon.h"

#include "Core/DungeonBuilder.h"
#include "Core/DungeonEventListener.h"
#include "Core/DungeonToolData.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Frameworks/LevelStreaming/DungeonLevelStreamingModel.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProvider.h"

#include "Components/BillboardComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY(DungeonLog);

TWeakObjectPtr<ADungeon> ADungeon::DungeonActor;

ADungeon::ADungeon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), bDrawDebugData(false)
{
	USceneComponent* SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "SceneRoot");
	SceneComponent->SetMobility(EComponentMobility::Static);
	RootComponent = SceneComponent;
	bBuildInProgress = false;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	Uid = FGuid::NewGuid();
	
	bReplicates = true;
	NetUpdateFrequency = 1;
	MinNetUpdateFrequency = 1;

	LevelStreamingModel = CreateDefaultSubobject<UDungeonLevelStreamingModel>(TEXT("LevelStreamingModel"));
	LevelStreamingModel->SetIsReplicated(true);

	CreateBuilderInstance(ObjectInitializer);

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));

	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> DungeonSpriteObject;
			FName ID_Dungeon;
			FText NAME_Dungeon;

			FConstructorStatics()
				: DungeonSpriteObject(TEXT("/DungeonArchitect/Core/Textures/S_DungeonActor"))
				  , ID_Dungeon(TEXT("Dungeon"))
				  , NAME_Dungeon(NSLOCTEXT("SpriteCategory", "Dungeon", "Dungeon"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		if (SpriteComponent)
		{
			SpriteComponent->Sprite = ConstructorStatics.DungeonSpriteObject.Get();
			SpriteComponent->SetRelativeScale3D(FVector(1, 1, 1));
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Dungeon;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Dungeon;
			SpriteComponent->SetupAttachment(RootComponent);
			SpriteComponent->Mobility = EComponentMobility::Static;
		}
	}
#endif //WITH_EDITORONLY_DATA
}

void ADungeon::BuildDungeon()
{
	UDungeonBuilder* DungeonBuilder = GetBuilder();
	if (!DungeonBuilder)
	{
		UE_LOG(DungeonLog, Error, TEXT("No builder created. Aborting dungeon build"));
		return;
	}

	if (DungeonBuilder->SupportsTheming() && Themes.Num() == 0)
	{
		UE_LOG(DungeonLog, Warning, TEXT("No themes defined. Aborting dungeon build"));
		return;
	}

	QueuedBuildTask = MakeShareable(new FAsyncTask<FDungeonBuilderTask>(GetWorld(), this));
	if (BuildTask.IsValid() && BuildTask->IsDone())
	{
		bBuildInProgress = false;
	}
}

void ADungeon::DestroyDungeon()
{
	DungeonUtils::FDungeonEventListenerNotifier::NotifyPreDungeonDestroy(this);

	UDungeonBuilder* DungeonBuilder = GetBuilder();

	// Destroy all actors that have the dungeon's unique id tag
	const FName DungeonTag = UDungeonModelHelper::GetDungeonIdTag(this);
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (ActorItr->ActorHasTag(DungeonTag) || ActorItr->ActorHasTag(UDungeonModelHelper::GenericDungeonIdTag))
		{
			ActorItr->Destroy();
		}
	}

	if (DungeonBuilder && !DungeonBuilder->SupportsTheming())
	{
		DungeonBuilder->DestroyNonThemedDungeon(GetModel(), GetConfig(), GetQuery(), this, GetWorld());
	}

	// Reset the model
	auto Model = GetModel();
	if (Model)
	{
		Model->Reset();
	}

	DungeonUtils::FDungeonEventListenerNotifier::NotifyDungeonDestroyed(this);

	QueuedBuildTask = nullptr;
	bBuildInProgress = false;
	if (LevelStreamingModel)
	{
		LevelStreamingModel->Release(GetWorld());
	}
}

UDungeonBuilder* ADungeon::SetBuilderClass(TSubclassOf<UDungeonBuilder> InBuilderClass)
{
	BuilderClass = InBuilderClass;
	CreateBuilderInstance();
	return Builder;
}

void ADungeon::BeginPlay()
{
	Super::BeginPlay();

	DungeonActor = this;
}

void ADungeon::Destroyed()
{
	Super::Destroyed();
}

void ADungeon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bBuildInProgress)
	{
		if (BuildTask.IsValid())
		{
			if (BuildTask->IsDone())
			{
				FDungeonBuilderTask& Task = BuildTask->GetTask();
				Task.RunGameThreadCommands();
				if (!Task.IsRunningGameThreadCommands())
				{
					PostDungeonBuild();
					bBuildInProgress = false;
					BuildTask = nullptr;
				}
			}
			else
			{
				//UE_LOG(DungeonLog, Warning, TEXT("Waiting for build to complete"));
			}
		}
		else
		{
			bBuildInProgress = false;
		}
	}

	if (!bBuildInProgress && QueuedBuildTask.IsValid())
	{
		BuildTask = QueuedBuildTask;
		QueuedBuildTask = nullptr;

		// Perform the layout generation in a separate thread
		auto DungeonBuilder = GetBuilder();
		bool backgroundTaskSupported = DungeonBuilder->SupportsBackgroundTask();
		if (backgroundTaskSupported)
		{
			BuildTask->StartBackgroundTask();
		}
		else
		{
			BuildTask->StartSynchronousTask();
		}
		bBuildInProgress = true;
	}

	if (bDrawDebugData)
	{
		auto DungeonBuilder = GetBuilder();
		auto World = GetWorld();
		if (DungeonBuilder && World)
		{
			DungeonBuilder->DrawDebugData(World);
		}
	}

	ProcessLevelStreaming();
}

#if WITH_EDITOR
void ADungeon::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	const FName PropertyName = (e.Property != nullptr) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ADungeon, BuilderClass))
	{
		CreateBuilderInstance();
	}

	Super::PostEditChangeProperty(e);
}

void ADungeon::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	if (DuplicateMode == EDuplicateMode::Normal)
	{
		Uid = FGuid::NewGuid();
	}

	Super::PostDuplicate(DuplicateMode);
}

#endif

void ADungeon::PostDungeonBuild()
{
	if (!BuildTask.IsValid()) return;
	MarkPackageDirty();

	// Cleanup the model for serialization
	GetModel()->Cleanup();

	OnDungeonBuildComplete.Broadcast(this);

	// Notify that the markers have been emitted
	DungeonUtils::FDungeonEventListenerNotifier::NotifyPostDungeonBuild(this);
}

void ADungeon::InitializeQueryObject()
{
	if (!Query)
	{
		UE_LOG(DungeonLog, Error, TEXT("Cannot initialize query object. Invalid state"));
		return;
	}
	Query->Initialize(GetConfig(), GetModel(), GetActorTransform());
}

void GetPlayerLocations(UWorld* World, TArray<FVector>& OutPlayerLocations)
{
}

void ADungeon::ProcessLevelStreaming() const
{
	if (LevelStreamingModel)
	{
		UWorld* World = GetWorld();
		if (World->IsGameWorld())
		{
			FDungeonLevelStreamer::Process(World, LevelStreamingConfig, LevelStreamingModel);
		}
	}
}

template <typename T>
T* CreateNewObject(const FObjectInitializer& ObjectInitializer, UObject* Outer, UClass* ImplClass)
{
	UObject* Instance = ObjectInitializer.CreateDefaultSubobject(Outer, FName(*ImplClass->GetName()), T::StaticClass(),
	                                                             ImplClass, true, false);
	return Cast<T>(Instance);
}

template <typename T>
T* CreateNewObject(UObject* Outer, UClass* ImplClass)
{
	return NewObject<T>(Outer, ImplClass, FName(*ImplClass->GetName()));
}

void ADungeon::CreateBuilderInstance()
{
	if (BuilderClass == nullptr)
	{
		// Set to the default builder
		BuilderClass = UDungeonBuilder::DefaultBuilderClass();
	}

	if (Builder == nullptr || !Builder->IsValidLowLevel() || Builder->GetClass() != BuilderClass)
	{
		Builder = CreateNewObject<UDungeonBuilder>(this, BuilderClass);
		DungeonModel = CreateNewObject<UDungeonModel>(this, Builder->GetModelClass());
		Config = CreateNewObject<UDungeonConfig>(this, Builder->GetConfigClass());
		ToolData = CreateNewObject<UDungeonToolData>(this, Builder->GetToolDataClass());
		Query = CreateNewObject<UDungeonQuery>(this, Builder->GetQueryClass());
		InitializeQueryObject();
	}

	LevelStreamingModel->Release(GetWorld());
}

void ADungeon::CreateBuilderInstance(const FObjectInitializer& ObjectInitializer)
{
	if (BuilderClass == nullptr)
	{
		// Set to the default builder
		BuilderClass = UDungeonBuilder::DefaultBuilderClass();
	}

	if (Builder == nullptr || !Builder->IsValidLowLevel() || Builder->GetClass() != BuilderClass)
	{
		Builder = CreateNewObject<UDungeonBuilder>(ObjectInitializer, this, BuilderClass);
		DungeonModel = CreateNewObject<UDungeonModel>(ObjectInitializer, this, Builder->GetModelClass());
		Config = CreateNewObject<UDungeonConfig>(ObjectInitializer, this, Builder->GetConfigClass());
		ToolData = CreateNewObject<UDungeonToolData>(ObjectInitializer, this, Builder->GetToolDataClass());
		Query = CreateNewObject<UDungeonQuery>(ObjectInitializer, this, Builder->GetQueryClass());
		InitializeQueryObject();
	}
}

FDungeonBuilderTask::FDungeonBuilderTask(UWorld* pWorld, ADungeon* pDungeon) :
	Dungeon(pDungeon), World(pWorld), bRunningGameThreadCommands(false)
{
	Dungeon->CreateBuilderInstance();
}

void FDungeonBuilderTask::DoWork()
{
	if (!Dungeon) return;
	UDungeonModel* Model = Dungeon->GetModel();
	if (!Model) return;

	// Notify pre-build dungeon event
	DungeonUtils::FDungeonEventListenerNotifier::NotifyPreDungeonBuild(Dungeon);

	if (Model->ShouldAutoResetOnBuild())
	{
		Dungeon->GetModel()->Reset();
	}

	UDungeonBuilder* DungeonBuilder = Dungeon->GetBuilder();
	SceneProvider = DungeonBuilder->CreateSceneProvider(Dungeon->Config, Dungeon, World);
	SceneProvider->SetBuildPriorityLocation(Dungeon->BuildPriorityLocation);

	if (DungeonBuilder->SupportsTheming())
	{
		DungeonBuilder->BuildDungeon(Dungeon, World);

		// Notify that the layout of the dungeon has been built
		DungeonUtils::FDungeonEventListenerNotifier::NotifyDungeonLayoutBuilt(Dungeon);

		DungeonBuilder->EmitDungeonMarkers();
		DungeonBuilder->EmitCustomMarkers(Dungeon->MarkerEmitters);
		DungeonBuilder->ProcessMarkerReplacementVolumes();

		// Spawn dungeon visual objects (meshes, lights, blueprints etc)
		DungeonBuilder->ApplyDungeonTheme(Dungeon->Themes, Dungeon->ClusterThemes, SceneProvider, World);
	}
	else
	{
		DungeonBuilder->BuildNonThemedDungeon(Dungeon, SceneProvider, World);
		DungeonUtils::FDungeonEventListenerNotifier::NotifyPostDungeonBuild(Dungeon);
	}

	// Start running the game thread commands
	bRunningGameThreadCommands = true;

	Dungeon->ForceNetUpdate();
}

void FDungeonBuilderTask::RunGameThreadCommands()
{
	if (SceneProvider.IsValid())
	{
		UDungeonConfig* Config = Dungeon->GetConfig();
		SceneProvider->RunGameThreadCommands(Config->MaxBuildTimePerFrameMs);
		bRunningGameThreadCommands = SceneProvider->IsRunningGameThreadCommands();
	}
	else
	{
		bRunningGameThreadCommands = false;
	}
}


UDungeonQuery* ADungeon::GetQuery()
{
	if (!Query || (Builder && Query->GetClass() != Builder->GetQueryClass()))
	{
		if (Builder)
		{
			Query = CreateNewObject<UDungeonQuery>(this, Builder->GetQueryClass());
			InitializeQueryObject();
		}
		else
		{
			UE_LOG(DungeonLog, Error,
			       TEXT("No builder created. Cannot fetch query object.  Rebuild the dungeon to fix this"));
		}
	}
	return Query;
}

#if WITH_EDITORONLY_DATA

/** Returns SpriteComponent subobject **/
UBillboardComponent* ADungeon::GetSpriteComponent() const { return SpriteComponent; }
#endif
