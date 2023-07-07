//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Isaac/IsaacDungeonModel.h"
#include "Frameworks/ThemeEngine/Rules/DungeonSelectorLogic.h"
#include "IsaacDungeonSelectorLogic.generated.h"

/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UIsaacDungeonSelectorLogic : public UDungeonSelectorLogic {
    GENERATED_BODY()

public:

    /** called when something enters the sphere component */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    bool SelectNode(UIsaacDungeonModel* Model);
    virtual bool SelectNode_Implementation(UIsaacDungeonModel* Model);


};

