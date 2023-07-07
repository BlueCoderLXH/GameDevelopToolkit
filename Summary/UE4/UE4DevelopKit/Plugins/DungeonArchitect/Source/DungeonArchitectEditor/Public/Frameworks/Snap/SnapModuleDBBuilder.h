//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"

#include "Engine/Level.h"
#include "Engine/LevelBounds.h"

#if WITH_EDITOR
#include "EditorLevelLibrary.h"
#endif

namespace SnapModuleDatabaseBuilder
{
	class FDefaultModulePolicy
	{
	public:
		FBox CalculateBounds(ULevel* Level) const
		{
			FBox LevelBounds = FBox(ForceInit);
			for (AActor* Actor : Level->Actors)
			{
				if (ALevelBounds* LevelBoundsActor = Cast<ALevelBounds>(Actor))
				{
					LevelBounds = LevelBoundsActor->GetComponentsBoundingBox();
					break;
				}
			}
			if (!LevelBounds.IsValid)
			{
				LevelBounds = ALevelBounds::CalculateLevelBounds(Level);
			}
			return LevelBounds;
		}

		template <typename TModuleItem>
		void Initialize(TModuleItem& ModuleItem, const ULevel* Level, const UObject* InModuleDB)
		{
		}

		template <typename TModuleItem>
		void PostProcess(TModuleItem& ModuleItem, const ULevel* Level)
		{
		}
	};

	template <typename TConnectionItem>
	class TDefaultConnectionPolicy
	{
	public:
		static void Build(ASnapConnectionActor* ConnectionActor, TConnectionItem& OutConnection)
		{
			OutConnection.ConnectionId = ConnectionActor->GetConnectionId();
			OutConnection.Transform = ConnectionActor->GetActorTransform();
			OutConnection.ConnectionInfo = ConnectionActor->ConnectionComponent->ConnectionInfo;
			OutConnection.ConnectionConstraint = ConnectionActor->ConnectionComponent->ConnectionConstraint;
		}
	};
}

template <typename TModuleItem, typename TConnectionItem, typename TModulePolicy, typename TConnectionPolicy>
class TSnapModuleDatabaseBuilder
{
public:
	static void Build(TArray<TModuleItem>& InModules, UObject* InModuleDB)
	{
		for (TModuleItem& Module : InModules)
		{
			UWorld* World = Module.Level.LoadSynchronous();

			if (World)
			{
				TArray<ULevel*> AllLevels;
				
				AllLevels.Add(World->PersistentLevel);

				// Enable StreamingLevel Check
				TArray<ULevelStreaming*> StreamingLevels = World->GetStreamingLevels();
				if (StreamingLevels.Num() > 0)
				{
					const FString LongPackageName = Module.Level.GetLongPackageName();
					
					const bool LoadResult = UEditorLevelLibrary::LoadLevel(LongPackageName);

					UE_LOG(LogTemp, Log, TEXT("[Dungeon] Load PersistentLevel:%s, LoadResult:%s"), *LongPackageName, LoadResult ? TEXT("True") : TEXT("False"));
				}
				
				for (auto StreamLevel : StreamingLevels)
				{
					auto SubLevel = StreamLevel->GetLoadedLevel();
					if (SubLevel)
					{
						AllLevels.Add(SubLevel);
					}
				}

				for (auto Level : AllLevels)
				{
					if (!Level)
					{
						continue;
					}
					
					Level->UpdateLevelComponents(false);

					TModulePolicy ModulePolicy;
					ModulePolicy.Initialize(Module, Level, InModuleDB);

					Module.Connections.Reset();
					for (AActor* Actor : Level->Actors)
					{
						// Update the connection actor list
						ASnapConnectionActor* ConnectionActor = Cast<ASnapConnectionActor>(Actor);
						if (ConnectionActor && ConnectionActor->ConnectionComponent)
						{
							TConnectionItem& Connection = Module.Connections.AddDefaulted_GetRef();
							TConnectionPolicy::Build(ConnectionActor, Connection);
						}
					}

					Module.ModuleBounds = ModulePolicy.CalculateBounds(Level);
					ModulePolicy.PostProcess(Module, Level);
				}	
			}
		}
	}
};
