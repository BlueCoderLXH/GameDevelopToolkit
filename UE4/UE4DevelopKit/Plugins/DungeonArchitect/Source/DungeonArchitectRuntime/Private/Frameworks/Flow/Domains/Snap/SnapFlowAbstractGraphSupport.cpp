//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Snap/SnapFlowAbstractGraphSupport.h"

#include "Core/Utils/MathUtils.h"
#include "Core/Utils/Stats.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_PathBuilderBase.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleDatabase.h"

//////////////////////////////////////// Snap Abstract Graph Node Group Generator //////////////////////////////////////////////

class UFANodeSnapDomainData;

FSnapFlowAGNodeGroupGenerator::FSnapFlowAGNodeGroupGenerator(const USnapGridFlowModuleDatabase* ModuleDB) {
	if (ModuleDB) {
		TMap<FIntVector, float> GroupWeights;
		TMap<FIntVector, int32> GroupCounts;
		for (const FSnapGridFlowModuleDatabaseItem& Module : ModuleDB->Modules) {
			float& Weight = GroupWeights.FindOrAdd(Module.NumChunks);
			Weight += Module.SelectionWeight;

			int32& Count = GroupCounts.FindOrAdd(Module.NumChunks);
			Count++;
		}

		// Average out the weights
		for (auto& Entry : GroupWeights) {
			const FIntVector& Key = Entry.Key;
			const int32 Count = GroupCounts[Key];
			Entry.Value /= Count;
		}

		for (auto& Entry : GroupWeights) {
			FSnapFlowAGNodeGroupSetting& Setting = GroupSettings.AddDefaulted_GetRef();
			Setting.Weight = Entry.Value;
			Setting.GroupSize = Entry.Key;
		}
	}
	else {
		FSnapFlowAGNodeGroupSetting& DefaultSetting = GroupSettings.AddDefaulted_GetRef();
		DefaultSetting.Weight = 1;
		DefaultSetting.GroupSize = FIntVector(1, 1, 1);
	}
}

void FSnapFlowAGNodeGroupGenerator::Generate(const FFlowAbstractGraphQuery& InGraphQuery, const UFlowAbstractNode* InCurrentNode, const FRandomStream& InRandom,
                                             const TSet<FGuid>& InVisited, TArray<FFlowAGPathNodeGroup>& OutGroups) const {
	class FLocalCoordBuilder {
	public:
		void GetCoords(const FIntVector& InGroupSize, TArray<FIntVector>& OutVolumeCoords, TArray<FIntVector>& OutSurfaceCoords) {
			TArray<FIntVector>* VolumeCoordsPtr = VolumeCoordsMap.Find(InGroupSize);
			TArray<FIntVector>* SurfaceCoordsPtr = SurfaceCoordsMap.Find(InGroupSize);

			if (VolumeCoordsPtr) {
				check(SurfaceCoordsPtr);
				OutVolumeCoords = *VolumeCoordsPtr;
				OutSurfaceCoords = *SurfaceCoordsPtr;
				return;
			}

			check (!SurfaceCoordsPtr);
			TArray<FIntVector>& VolumeCoords = VolumeCoordsMap.FindOrAdd(InGroupSize);
			TArray<FIntVector>& SurfaceCoords = SurfaceCoordsMap.FindOrAdd(InGroupSize);
			
			for (int dz = 0; dz < InGroupSize.Z; dz++) {
				for (int dy = 0; dy < InGroupSize.Y; dy++) {
					for (int dx = 0; dx < InGroupSize.X; dx++) {
						VolumeCoords.Add(FIntVector(dx, dy, dz));
						if (dx == 0 || dx == InGroupSize.X - 1 ||
	                            dy == 0 || dy == InGroupSize.Y - 1 ||
	                            dz == 0 || dz == InGroupSize.Z - 1) {
							SurfaceCoords.Add(FIntVector(dx, dy, dz));
                        }
					}
				}
			}
		}
	
	private:
		TMap<FIntVector, TArray<FIntVector>> VolumeCoordsMap;
		TMap<FIntVector, TArray<FIntVector>> SurfaceCoordsMap;
	};
	
	const UFlowAbstractNode* Node = InCurrentNode;
	if (!Node) {
		return;
	}

	if (GroupSettings.Num() == 0) {
		const FNullFlowAGNodeGroupGenerator NullGenerator;
		NullGenerator.Generate(InGraphQuery, Node, InRandom, InVisited, OutGroups);
		return;
	}

	TArray<FSnapFlowAGNodeGroupSetting> WeightedGroupSettings = GroupSettings;
	for (FSnapFlowAGNodeGroupSetting& Settings : WeightedGroupSettings) {
		// Add randomness to similar weighted objects 
		Settings.Weight += InRandom.FRand() * 0.0001f;
	}

	static FLocalCoordBuilder CoordBuilder;
	
	for (const FSnapFlowAGNodeGroupSetting& GroupSetting : WeightedGroupSettings) {
		TArray<FIntVector> LocalSurfaceCoords;
		TArray<FIntVector> LocalVolumeCoords;
		CoordBuilder.GetCoords(GroupSetting.GroupSize, LocalVolumeCoords, LocalSurfaceCoords);
		
		for (const FIntVector& LocalSurfaceCoord : LocalSurfaceCoords) {
			bool bValid = true;
			FIntVector BaseCoord = FMathUtils::ToIntVector(Node->Coord, true) - LocalSurfaceCoord;
			for (const FIntVector& LocalVolumeCoord : LocalVolumeCoords) {
				FIntVector GroupNodeCoord = BaseCoord + LocalVolumeCoord;
				

				const UFlowAbstractNode* TestNode = InGraphQuery.GetNodeObjectAtCoord(GroupNodeCoord);
 				if (!TestNode || InVisited.Contains(TestNode->NodeId) || TestNode->bActive) {
					bValid = false;
					break;
				}
			}

			if (bValid) {
				// Add this group
				FFlowAGPathNodeGroup& NewGroup = OutGroups.AddDefaulted_GetRef();
				NewGroup.bIsGroup = true;
				NewGroup.Weight = GroupSetting.Weight;
				for (const FIntVector& LocalVolumeCoord : LocalVolumeCoords) {
					FIntVector NodeCoord = BaseCoord + LocalVolumeCoord;
					const UFlowAbstractNode* GroupNode = InGraphQuery.GetNodeObjectAtCoord(NodeCoord);
					NewGroup.GroupNodes.Add(GroupNode->NodeId);
				}
				for (const FIntVector& SurfCoord : LocalSurfaceCoords) {
					FIntVector NodeCoord = BaseCoord + SurfCoord;
					const UFlowAbstractNode* GroupNode = InGraphQuery.GetNodeObjectAtCoord(NodeCoord);
					NewGroup.GroupEdgeNodes.Add(GroupNode->NodeId);
				}
			}
		}
	}
}

int32 FSnapFlowAGNodeGroupGenerator::GetMinNodeGroupSize() const {
	if (GroupSettings.Num() == 0) return 1;

	int32 MinGroupSize = MAX_int32;
	for (const FSnapFlowAGNodeGroupSetting& GroupSetting : GroupSettings) {
		const int32 GroupSize = GroupSetting.GroupSize.X * GroupSetting.GroupSize.Y * GroupSetting.GroupSize.Z;
		MinGroupSize = FMath::Min(MinGroupSize, GroupSize);
	}
	return MinGroupSize;
}


//////////////////////////////////////// Snap Abstract Graph Constraint System //////////////////////////////////////////////

FSnapGridFlowAbstractGraphConstraints::FSnapGridFlowAbstractGraphConstraints(USnapGridFlowModuleDatabase* InModuleDatabase)
	: ModuleDatabase(InModuleDatabase) {
}


bool FSnapGridFlowAbstractGraphConstraints::IsValid(const FFlowAbstractGraphQuery& InGraphQuery, const FFlowAGPathNodeGroup& Group,
                                                    const TArray<FFAGConstraintsLink>& IncomingNodes, const TArray<FName>& InAllowedCategories) const {
	if (Group.GroupEdgeNodes.Num() == 0 || Group.GroupNodes.Num() == 0) return false;

	// Build the input node assembly
	FSGFModuleAssembly Assembly;
	FSGFModuleAssemblyBuilder::Build(InGraphQuery, Group, IncomingNodes, Assembly);

	if (ModuleDatabase.IsValid()) {
		for (const FSnapGridFlowModuleDatabaseItem& Module : ModuleDatabase->Modules) {
			if (InAllowedCategories.Contains(Module.Category)) {
				const int32 NumRotatedAssemblies = Module.RotatedAssemblies.Num();
				for (int StepIdx = 0; StepIdx < NumRotatedAssemblies; StepIdx++) {
					const FSGFModuleAssembly& RegisteredAssembly = Module.RotatedAssemblies[StepIdx];
					TArray<FSGFModuleAssemblySideCell> DoorIndices;
					
					if (RegisteredAssembly.CanFit(Assembly, DoorIndices)) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool FSnapGridFlowAbstractGraphConstraints::IsValid(const FFlowAbstractGraphQuery& InGraphQuery, const UFlowAbstractNode* Node,
                                                    const TArray<const UFlowAbstractNode*>& IncomingNodes,
                                                    const TArray<TWeakObjectPtr<UObject>>& InTaskExtenders) {
	
	UFlowAbstractGraphBase* Graph = InGraphQuery.GetGraph<UFlowAbstractGraphBase>();
	if (!Graph) return false;
	check(Node && Node->PathIndex != INDEX_NONE);


	TSet<const UFlowAbstractNode*> AllIncomingNodes(IncomingNodes);
	for (const FGuid& IncomingNode : Graph->GetIncomingNodes(Node->NodeId)) {
		AllIncomingNodes.Add(InGraphQuery.GetNode(IncomingNode));
	}

	FFlowAGPathNodeGroup Group;
	TArray<FFAGConstraintsLink> ConstraintLinks;
	BuildNodeGroup(InGraphQuery, Node, AllIncomingNodes.Array(), Group, ConstraintLinks);

	TArray<FName> AllowedCategories;
	UFANodeSnapDomainData* NodeSnapData = Node->FindDomainData<UFANodeSnapDomainData>();
	if (!NodeSnapData) {
		return false;
	}

	return IsValid(InGraphQuery, Group, ConstraintLinks, NodeSnapData->ModuleCategories); 
}

bool FSnapGridFlowAbstractGraphConstraints::IsValid(const FFlowAbstractGraphQuery& InGraphQuery, const FFlowAGPathNodeGroup& Group, int32 PathIndex,
                                                    int32 PathLength,
                                                    const TArray<FFAGConstraintsLink>& IncomingNodes, const TArray<TWeakObjectPtr<UObject>>& InTaskExtenders) {
	TArray<FName> AllowedCategories;
	for (TWeakObjectPtr<UObject> Extender : InTaskExtenders) {
		if (USnapFlowAGTaskExtender* SnapFlowTaskExtender = Cast<USnapFlowAGTaskExtender>(Extender)) {
			AllowedCategories = SnapFlowTaskExtender->GetCategoriesAtNode(PathIndex, PathLength);
			break;
		}
	}

	return IsValid(InGraphQuery, Group, IncomingNodes, AllowedCategories);
}

void FSnapGridFlowAbstractGraphConstraints::BuildNodeGroup(const FFlowAbstractGraphQuery& InGraphQuery,
                                                           const UFlowAbstractNode* InNode, const TArray<const UFlowAbstractNode*>& InIncomingNodes,
                                                           FFlowAGPathNodeGroup& OutGroup, TArray<FFAGConstraintsLink>& OutConstraintLinks) {
	UFlowAbstractGraphBase* Graph = InGraphQuery.GetGraph<UFlowAbstractGraphBase>();
	if (!Graph) return;

	FIntVector MinCoord, MaxCoord;
	OutGroup = FFlowAGPathNodeGroup();
	if (InNode->MergedCompositeNodes.Num() <= 1) {
		OutGroup.bIsGroup = false;
		OutGroup.GroupNodes.Add(InNode->NodeId);
		OutGroup.GroupEdgeNodes.Add(InNode->NodeId);
		MinCoord = MaxCoord = FMathUtils::ToIntVector(InNode->Coord, true);
	}
	else {
		OutGroup.bIsGroup = true;
		FVector MinCoordF = InNode->MergedCompositeNodes[0]->Coord;
		FVector MaxCoordF = MinCoordF;

		for (UFlowAbstractNode* const SubNode : InNode->MergedCompositeNodes) {
			MinCoordF = MinCoordF.ComponentMin(SubNode->Coord);
			MaxCoordF = MaxCoordF.ComponentMax(SubNode->Coord);
			OutGroup.GroupNodes.Add(SubNode->NodeId);
		}
		MinCoord = FMathUtils::ToIntVector(MinCoordF, true);
		MaxCoord = FMathUtils::ToIntVector(MaxCoordF, true);

		for (UFlowAbstractNode* const SubNode : InNode->MergedCompositeNodes) {
			const FIntVector Coord = FMathUtils::ToIntVector(SubNode->Coord, true);
			if (Coord.X == MinCoord.X || Coord.Y == MinCoord.Y || Coord.Z == MinCoord.Z ||
				Coord.X == MaxCoord.X || Coord.Y == MaxCoord.Y || Coord.Z == MaxCoord.Z) {
				OutGroup.GroupEdgeNodes.Add(SubNode->NodeId);
			}
		}
	}

	for (UFlowAbstractLink* Link : Graph->GraphLinks) {
		if (Link->Type == EFlowAbstractLinkType::Unconnected) continue;

		FGuid Source = Link->SourceSubNode.IsValid() ? Link->SourceSubNode : Link->Source;
		FGuid Destination = Link->DestinationSubNode.IsValid() ? Link->DestinationSubNode : Link->Destination;

		const bool bHostsSource = OutGroup.GroupNodes.Contains(Source);
		const bool bHostsDest = OutGroup.GroupNodes.Contains(Destination);
		if (!bHostsSource && !bHostsDest) continue;
		if (bHostsSource && bHostsDest) continue;

		if (bHostsSource) {
			if (OutGroup.GroupEdgeNodes.Contains(Source)) {
				UFlowAbstractNode* SourceNode = InGraphQuery.GetNode(Source);
				if (!SourceNode) SourceNode = InGraphQuery.GetSubNode(Source);
				UFlowAbstractNode* DestinationNode = InGraphQuery.GetNode(Destination);
				if (!DestinationNode) DestinationNode = InGraphQuery.GetSubNode(Destination);
				if (SourceNode && DestinationNode) {
					OutConstraintLinks.Add(FFAGConstraintsLink(SourceNode, DestinationNode));
				}
			}
		}
		else if (bHostsDest) {
			if (OutGroup.GroupEdgeNodes.Contains(Destination)) {
				UFlowAbstractNode* SourceNode = InGraphQuery.GetNode(Source);
				if (!SourceNode) SourceNode = InGraphQuery.GetSubNode(Source);
				UFlowAbstractNode* DestinationNode = InGraphQuery.GetNode(Destination);
				if (!DestinationNode) DestinationNode = InGraphQuery.GetSubNode(Destination);
				if (SourceNode && DestinationNode) {
					OutConstraintLinks.Add(FFAGConstraintsLink(DestinationNode, SourceNode));
				}
			}
		}
	}

	TMap<FIntVector, UFlowAbstractNode*> NodeByCoords;
	for (UFlowAbstractNode* GraphNode : Graph->GraphNodes) {
		if (GraphNode->MergedCompositeNodes.Num() > 0) {
			for (UFlowAbstractNode* SubNode : GraphNode->MergedCompositeNodes) {
				FIntVector Coord = FMathUtils::ToIntVector(SubNode->Coord, true);
				UFlowAbstractNode*& NodeRef = NodeByCoords.FindOrAdd(Coord);
				NodeRef = SubNode;
			}
		}
		else {
			FIntVector Coord = FMathUtils::ToIntVector(GraphNode->Coord, true);
			UFlowAbstractNode*& NodeRef = NodeByCoords.FindOrAdd(Coord);
			NodeRef = GraphNode;
		}
	}

	for (const UFlowAbstractNode* const IncomingNode : InIncomingNodes) {
		if (!IncomingNode) continue;
		FIntVector InnerCoord = FMathUtils::ToIntVector(IncomingNode->Coord, true);
		InnerCoord.X = FMath::Clamp(InnerCoord.X, MinCoord.X, MaxCoord.X);
		InnerCoord.Y = FMath::Clamp(InnerCoord.Y, MinCoord.Y, MaxCoord.Y);
		InnerCoord.Z = FMath::Clamp(InnerCoord.Z, MinCoord.Z, MaxCoord.Z);
		UFlowAbstractNode** InnerNodePtr = NodeByCoords.Find(InnerCoord);
		if (InnerNodePtr) {
			UFlowAbstractNode* InnerNode = *InnerNodePtr;
			OutConstraintLinks.Add(FFAGConstraintsLink(InnerNode, IncomingNode));
		}
	}
}

//////////////////////////////////////// Snap Abstract Graph Task Extender /////////////////////////////////////////////

TArray<FName> USnapFlowAGTaskExtender::GetCategoriesAtNode(int32 PathIndex, int32 PathLength) {
	TArray<FName> TargetCategories = ModuleCategories;

	if (ModuleCategoryOverrideMethod == ESnapFlowAGTaskModuleCategoryOverrideMethod::Blueprint) {
		// Check if a selection override bp wants to override the category list
		for (USnapFlowNodeCategorySelectionOverride* SelectionOverride : CategoryOverrideLogic) {
			TArray<FName> CategoryOverrides;
			if (SelectionOverride && SelectionOverride->TryOverrideCategories(PathIndex, PathLength, ModuleCategories, CategoryOverrides)) {
				TargetCategories = CategoryOverrides;
				break;
			}
		}
	}
	else if (ModuleCategoryOverrideMethod == ESnapFlowAGTaskModuleCategoryOverrideMethod::StartEnd) {
		if (PathIndex == 0 && StartNodeCategoryOverride.Num() > 0) {
			TargetCategories = StartNodeCategoryOverride;
		}
		else if (PathIndex == PathLength - 1 && EndNodeCategoryOverride.Num() > 0) {
			TargetCategories = EndNodeCategoryOverride;
		}
	}

	return TargetCategories;
}

void USnapFlowAGTaskExtender::ExtendNode(UFlowAbstractNode* Node) {
	UFANodeSnapDomainData* NodeDomainData = Node->FindOrAddDomainData<UFANodeSnapDomainData>();
	NodeDomainData->ModuleCategories = GetCategoriesAtNode(Node->PathIndex, Node->PathLength);
}

#if WITH_EDITOR
FString USnapFlowAGTaskExtender::GetDetailsPanelCategoryName() const {
	return "Snap";
}
#endif //WITH_EDITOR
