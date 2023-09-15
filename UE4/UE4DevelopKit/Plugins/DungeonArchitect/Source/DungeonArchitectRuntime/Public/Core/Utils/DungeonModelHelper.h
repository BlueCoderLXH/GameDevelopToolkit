//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Dungeon.h"
#include "Core/Utils/Rectangle.h"

#include "EngineUtils.h"
#include "DungeonModelHelper.generated.h"

class ADungeon;

/**
 * 
 */
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UDungeonModelHelper : public UObject {
    GENERATED_UCLASS_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static FVector MakeVector(const FIntVector& In, float scale = 1);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static FIntVector MakeIntVector(const FVector& In);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static void ExpandBounds(const FRectangle& Bounds, int32 Size, FRectangle& Result);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static void GetCenterExtent(const FRectangle& Rectangle, FVector& Center, FVector& Extent);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static void GetRectBorderPoints(const FRectangle& Rectangle, TArray<FIntVector>& BorderPoints);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static void GetRectAreaPoints(const FRectangle& Rectangle, TArray<FIntVector>& BorderPoints);

    template <typename T>
    static TArray<T*> CreateInstances(TArray<TSubclassOf<T>> TClasses) {
        TArray<T*> Result;
        for (TSubclassOf<T> TClass : TClasses) {
            if (!TClass) continue;
            T* t = NewObject<T>(static_cast<UObject*>(GetTransientPackage()), TClass);
            if (t) {
                Result.Add(t);
            }
        }
        return Result;
    }

    template <typename T>
    static void CloneUObjectArray(UObject* Outer, const TArray<T*>& SourceList, TArray<T*>& DestList) {
        DestList.Reset();
        for (T* Source : SourceList) {
            if (!Source) continue;
            T* Clone = NewObject<T>(Outer, Source->GetClass(), NAME_None, RF_NoFlags, Source);
            DestList.Add(Clone);
        }
    }

    static const FName GenericDungeonIdTag;
    static FName GetDungeonIdTag(ADungeon* Dungeon);
    static bool LineIntersectsRect(const FIntVector& p1, const FIntVector& p2, const FRectangle& r);
    static bool LineIntersectsLine(const FIntVector& l1p1, const FIntVector& l1p2, const FIntVector& l2p1,
                                   const FIntVector& l2p2);

    static bool GetNodeId(const FName& DungeonTag, AActor* Actor, FName& OutNodeId);

    template <typename T>
    static TArray<T*> GetActors(UWorld* World) {
        TArray<T*> Result;
        if (World) {
            for (TActorIterator<T> It(World); It; ++It) {
                T* Actor = *It;
                Result.Add(Actor);
            }
        }
        return Result;
    }

    template <typename T>
    static TArray<T*> GetVolumes(ADungeon* Dungeon) {
        UWorld* World = Dungeon ? Dungeon->GetWorld() : nullptr;
        TArray<T*> Result;
        if (World) {
            for (TActorIterator<T> It(World); It; ++It) {
                T* Volume = *It;
                if (Volume->Dungeon == Dungeon) {
                    Result.Add(Volume);
                }
            }
        }
        return Result;
    }

};

