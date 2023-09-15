//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/FloorPlan/FloorPlanConfig.h"


DEFINE_LOG_CATEGORY(FloorPlanConfigLog);

UFloorPlanConfig::UFloorPlanConfig(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
      , BuildingSize(15, 20, 1)
      , GridSize(400, 400, 200)
      , MinRoomSize(3)
      , MaxRoomSize(5)
      , HallWidth(1)
      , MinRoomChunkArea(25)
      , RoomSplitProbabilityOffset(0.25f) {
}

#if WITH_EDITOR
void UFloorPlanConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property) {
		const FName PropertyName = PropertyChangedEvent.Property->GetFName(); 
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UFloorPlanConfig, MinRoomSize)) {
			MaxRoomSize = FMath::Max(MinRoomSize, MaxRoomSize);
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UFloorPlanConfig, MaxRoomSize)) {
			MinRoomSize = FMath::Min(MinRoomSize, MaxRoomSize);
		} 
	}
}
#endif // WITH_EDITOR

