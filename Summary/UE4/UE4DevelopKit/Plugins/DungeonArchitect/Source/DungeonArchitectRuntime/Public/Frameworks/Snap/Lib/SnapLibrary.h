//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/PermutationEngine.h"

enum class ESnapConnectionConstraint : unsigned char;
class USnapConnectionInfo;

namespace SnapLib
{
	typedef TSharedPtr<struct FModuleNode> FModuleNodePtr;
	typedef TWeakPtr<struct FModuleNode> FModuleNodeWeakPtr;
	typedef TSharedPtr<struct FModuleDoor> FModuleDoorPtr;
	typedef TWeakPtr<struct FModuleDoor> FModuleDoorWeakPtr;

	typedef TSharedPtr<class IModuleDatabaseItem> IModuleDatabaseItemPtr;
	class FDiagnostics;

	struct FSnapNegationVolumeState
	{
		FBox Bounds;
		bool bInverse = false;
	};

	struct FModuleDoor
	{
		FGuid ConnectionId;

		// The transform of the door relative to the module
		FTransform LocalTransform;

		// The connection info object defined in the door connection asset
		USnapConnectionInfo* ConnectionInfo;

		// The connection constraint (magnet, male-female etc)
		ESnapConnectionConstraint ConnectionConstraint;

		// The module that hosts this door
		FModuleNodePtr Owner;

		// The other door that is connected to this door
		FModuleDoorPtr ConnectedDoor;
	};

	struct FModuleNode
	{
		FName Category;
		
		FGuid ModuleInstanceId;
		FTransform WorldTransform = FTransform::Identity;
		IModuleDatabaseItemPtr ModuleDBItem;

		// The doors in this module
		TArray<FModuleDoorPtr> Doors;

		FModuleDoorPtr Incoming;
		TSet<FModuleDoorPtr> Outgoing;

		uint32 NetworkId = 0;

		FBox GetModuleBounds() const;
	};

	struct FGrowthInputState
	{
		TSet<FGuid> VisitedNodes;
		TArray<FBox> OcclusionStack;
		FModuleDoorPtr RemoteIncomingDoor;
		int32 RemoteIncomingDoorIndex = -1;
	};

	enum class FGrowthResultType
	{
		Success,
		FailBranch,
		FailHalt
	};

	struct FGrowthResult
	{
		FGrowthResultType SuccessType = FGrowthResultType::FailBranch;
		FModuleNodePtr Node;
		FModuleDoorPtr IncomingDoor;
		TSet<FGuid> BranchVisited;
		TArray<FBox> BranchOcclusion;
	};

	struct FGrowthStaticState
	{
		FRandomStream Random;
		float BoundsContraction = 0;
		FTransform DungeonBaseTransform = FTransform::Identity;
		double StartTimeSecs = 0;
		double MaxProcessingTimeSecs = 0;
		bool bAllowModuleRotations = true;
		TArray<FSnapNegationVolumeState> NegationVolumes;
		TSharedPtr<SnapLib::FDiagnostics> Diagnostics;
	};

	struct FGrowthSharedState
	{
		TMap<FGuid, FModuleNodeWeakPtr> CachedModuleNodes;
	};

	typedef TSharedPtr<class ISnapGraphNode> ISnapGraphNodePtr;

	class DUNGEONARCHITECTRUNTIME_API ISnapGraphNode
	{
	public:
		virtual ~ISnapGraphNode()
		{
		}

		virtual FGuid GetNodeID() const = 0;
		virtual FName GetCategory() const = 0;
		virtual TArray<ISnapGraphNodePtr> GetOutgoingNodes(const FGuid& IncomingNodeId) const = 0;

		int32 Compare(const ISnapGraphNodePtr Other) const
		{
			return GetCategory().CompareIndexes(Other->GetCategory());
		}
	};

	class FBranchGrowthPermutations
	{
	public:
		FBranchGrowthPermutations(const TArray<int32>& InOutgoingDoors,
		                          const TArray<ISnapGraphNodePtr> InOutgoingNodes);
		void Execute(TArray<int32>& OutDoors, TArray<ISnapGraphNodePtr>& OutNodes);
		FORCEINLINE bool CanRun() const { return Combination->CanRun() || Permutation->CanPermute(); }

	private:
		TSharedPtr<FPermutation<ISnapGraphNodePtr>> Permutation;
		TSharedPtr<FCombination<int32>> Combination;
		TArray<int32> OutgoingDoors;
		TArray<ISnapGraphNodePtr> OutgoingNodes;
		TArray<int32> CurrentSelection;
	};

	class IModuleDatabaseItem : public TSharedFromThis<IModuleDatabaseItem>
	{
	public:
		virtual ~IModuleDatabaseItem()
		{
		}

		virtual FBox GetBounds() = 0;
		virtual FName GetCategory() = 0;
		virtual TSoftObjectPtr<UWorld> GetLevel() = 0;
		virtual TSoftObjectPtr<UWorld> GetLogicLevel() = 0;
		virtual FModuleNodePtr CreateModuleNode(const FGuid& InNodeId) = 0;

	protected:
	};

	class IModuleDatabase
	{
	public:
		virtual ~IModuleDatabase()
		{
		}

		const TArray<IModuleDatabaseItemPtr>& GetCategoryModules(const FName& ModuleCategory)
		{
			return ModulesByCategory.FindOrAdd(ModuleCategory);
		}

	protected:
		TMap<FName, TArray<IModuleDatabaseItemPtr>> ModulesByCategory;
	};

	typedef TSharedPtr<IModuleDatabase> IModuleDatabasePtr;


	class DUNGEONARCHITECTRUNTIME_API FSnapGraphGenerator
	{
	public:
		FSnapGraphGenerator(IModuleDatabasePtr InModuleDatabase, const FGrowthStaticState& InStaticState);

		virtual ~FSnapGraphGenerator()
		{
		}

		FModuleNodePtr Generate(ISnapGraphNodePtr StartNode);

	protected:
		virtual bool ModuleOccludes(const FModuleNodePtr ModuleNode, SnapLib::ISnapGraphNodePtr MissionNode,
		                            const TArray<FBox>& OcclusionList);
		virtual TArray<FTransform> GetStartingNodeTransforms(SnapLib::FModuleNodePtr ModuleNode,
		                                                     SnapLib::ISnapGraphNodePtr MissionNode);

	private:
		void GrowNode(ISnapGraphNodePtr MissionNode, const FGrowthInputState& InputState, FGrowthResult& OutResult);
		static void AssignNetworkNodeIds(FModuleNodePtr Node);
		static FModuleNodePtr GetConnectedModule(FModuleDoorPtr Door);
		bool GetDoorFitConfiguration(FModuleDoorPtr RemoteDoor, FModuleDoorPtr DoorToFit,
		                             bool bAllowModuleRotation, TArray<FTransform>& OutNewTransforms);
		bool CanConnectDoors(FModuleDoorPtr A, FModuleDoorPtr B, bool bAllowRotation);

	protected:
		IModuleDatabasePtr ModuleDatabase;
		const FGrowthStaticState StaticState;
		FGrowthSharedState SharedState;
	};
}
