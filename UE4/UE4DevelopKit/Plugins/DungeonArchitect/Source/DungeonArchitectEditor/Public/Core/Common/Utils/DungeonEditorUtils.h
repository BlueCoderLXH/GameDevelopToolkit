//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Notifications/SNotificationList.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDungeonEditorUtils, Log, All);

class ADungeon;

class DUNGEONARCHITECTEDITOR_API FDungeonEditorUtils {
public:
    static ADungeon* GetDungeonActorFromLevelViewport();

    template <typename T>
    static T* GetBuilderObject(IDetailLayoutBuilder* DetailBuilder) {
        TArray<TWeakObjectPtr<UObject>> OutObjects;
        DetailBuilder->GetObjectsBeingCustomized(OutObjects);
        T* Obj = nullptr;
        if (OutObjects.Num() > 0) {
            Obj = Cast<T>(OutObjects[0].Get());
        }
        return Obj;
    }

    static void ShowNotification(FText Text, SNotificationItem::ECompletionState State = SNotificationItem::CS_Fail);
    static void SwitchToRealtimeMode();
    static void CreateDungeonItemFolder(ADungeon* Dungeon);
    static void BuildDungeon(ADungeon* Dungeon);

private:
    static void CollapseDungeonItemFolder(ADungeon* Dungeon);
};


struct DUNGEONARCHITECTEDITOR_API FAssetPackageInfo : public FGCObject {
    UObject* Asset;

    //The package that contains the asset
    UPackage* Package;

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override {
        Collector.AddReferencedObject(Asset);
        Collector.AddReferencedObject(Package);
    }
};

class DUNGEONARCHITECTEDITOR_API FDungeonAssetUtils {
public:
    static FAssetPackageInfo DuplicateAsset(UObject* SourceAsset, const FString& TargetPackageName,
                                            const FString& TargetObjectName);
    static void SaveAsset(const FAssetPackageInfo& Package);
};

class FWidgetFlasher {
public:
    FWidgetFlasher()
        : FlashColor(FLinearColor::Red)
          , FlashDuration(0.5f) {
    }

    void Flash() {
        if (Widget.IsValid()) {
            const float Duration = 0.5f;
            FlashCurve = FCurveSequence(0, Duration, ECurveEaseFunction::Linear);
            FlashCurve.Play(Widget.Pin().ToSharedRef());
        }
    }

    bool IsFlashing() const {
        return FlashCurve.IsPlaying();
    }

    FLinearColor GetFlashCurveColor() const {
        FLinearColor StartColor = FColor(32, 32, 32);
        FLinearColor EndColor = FlashColor;
        const float Lerp = FlashCurve.GetLerp();
        const float t = FMath::Sin(Lerp * PI);
        return FMath::Lerp(StartColor, EndColor, t);
    }

public:
    TWeakPtr<SWidget> Widget;
    FCurveSequence FlashCurve;
    FLinearColor FlashColor;
    float FlashDuration;
};

