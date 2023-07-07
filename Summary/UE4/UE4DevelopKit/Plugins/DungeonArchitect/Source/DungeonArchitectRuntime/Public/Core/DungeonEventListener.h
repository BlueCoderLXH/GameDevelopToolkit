//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonBuilder.h"
#include "Core/DungeonModel.h"
#include "DungeonEventListener.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(DungeonEventListenerLog, Log, All);

/**
* Implement this class in blueprint (or C++) to emit your own custom markers in the scene
*/
UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, abstract, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UDungeonEventListener : public UObject {
    GENERATED_BODY()

public:
    UDungeonEventListener(const FObjectInitializer& ObjectInitializer);

    /** Called before the dungeon is built */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void OnPreDungeonBuild(ADungeon* Dungeon);
    virtual void OnPreDungeonBuild_Implementation(ADungeon* Dungeon);

    /** Called after the layout is generated in memory */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void OnDungeonLayoutBuilt(ADungeon* Dungeon);
    virtual void OnDungeonLayoutBuilt_Implementation(ADungeon* Dungeon);

    /** Called after all the markers are emitted into the scene */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void OnMarkersEmitted(ADungeon* Dungeon, UPARAM(ref) TArray<FDungeonMarkerInfo>& MarkerList,
                          TArray<FDungeonMarkerInfo>& MarkerListRef);
    virtual void OnMarkersEmitted_Implementation(ADungeon* Dungeon, UPARAM(ref) TArray<FDungeonMarkerInfo>& MarkerList,
                                                 TArray<FDungeonMarkerInfo>& MarkerListRef);

    /** Called after the dungeon is completely built */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void OnPostDungeonBuild(ADungeon* Dungeon);
    virtual void OnPostDungeonBuild_Implementation(ADungeon* Dungeon);

    /** Called before the dungeon is about to be destroyed */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void OnPreDungeonDestroy(ADungeon* Dungeon);
    virtual void OnPreDungeonDestroy_Implementation(ADungeon* Dungeon);

    /** Called after the dungeon has been destroyed */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void OnDungeonDestroyed(ADungeon* Dungeon);
    virtual void OnDungeonDestroyed_Implementation(ADungeon* Dungeon);

};

namespace DungeonUtils {
    class FDungeonEventListenerNotifier {
    public:
        static void NotifyPreDungeonBuild(ADungeon* Dungeon);
        static void NotifyDungeonLayoutBuilt(ADungeon* Dungeon);
        static void NotifyMarkersEmitted(ADungeon* Dungeon, TArray<FDungeonMarkerInfo>& PropSockets);
        static void NotifyPostDungeonBuild(ADungeon* Dungeon);
        static void NotifyPreDungeonDestroy(ADungeon* Dungeon);
        static void NotifyDungeonDestroyed(ADungeon* Dungeon);
    };
}

