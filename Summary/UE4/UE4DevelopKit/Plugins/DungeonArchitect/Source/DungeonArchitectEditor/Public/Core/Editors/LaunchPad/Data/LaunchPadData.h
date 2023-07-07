//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "LaunchPadData.generated.h"

/////////////////// Categories /////////////////// 
USTRUCT()
struct FLaunchPadCategoryItem {
    GENERATED_BODY()

    UPROPERTY()
    FString Page;

    UPROPERTY()
    FString Title;
};

USTRUCT()
struct FLaunchPadCategories {
    GENERATED_BODY()

    UPROPERTY()
    TArray<FLaunchPadCategoryItem> Items;
};

/////////////////// Page /////////////////// 

UENUM()
enum class ELaunchPagePageLayout : uint8 {
    CardGrid,
    Details,
    News
};


USTRUCT()
struct FLaunchPadPageHeader {
    GENERATED_BODY()

    UPROPERTY()
    FString Title;

    UPROPERTY()
    ELaunchPagePageLayout Layout;
};

/////////////////// Page Layout: CardGrid /////////////////// 

USTRUCT()
struct FLaunchPadPageLayout_CardGrid_CardInfo {
    GENERATED_BODY()

    UPROPERTY()
    FString Title;

    UPROPERTY()
    FString Desc;

    UPROPERTY()
    FString Image;

    UPROPERTY()
    FString Link;

    UPROPERTY()
    FString Url;
};

USTRUCT()
struct FLaunchPadPageLayout_CardGrid_Category {
    GENERATED_BODY()

    UPROPERTY()
    FString Category;

    UPROPERTY()
    TArray<FLaunchPadPageLayout_CardGrid_CardInfo> Cards;
};

USTRUCT()
struct FLaunchPadPageLayout_CardGrid {
    GENERATED_BODY()

    UPROPERTY()
    FString Title;

    UPROPERTY()
    FString Description;

    UPROPERTY()
    bool ShowCategoryTitles = true;

    UPROPERTY()
    int32 ImageWidth = 180;

    UPROPERTY()
    int32 ImageHeight = 100;

    UPROPERTY()
    int32 CardHeight = 185;

    UPROPERTY()
    int32 CardPadding = 2;

    UPROPERTY()
    TArray<FLaunchPadPageLayout_CardGrid_Category> Categories;
};

UENUM()
enum class ELaunchPadActionType : uint8 {
    None,
    OpenFolder,
    OpenScene,
    OpenTheme,
    OpenSnapFlow,
    OpenGridFlow,
    CloneScene,
    CloneSceneAndBuild,
    CloneTheme,
    CloneSnapFlow,
    CloneGridFlow,
    CloneSnapGridFlow,
    Documentation,
    LauncherURL,
    AddStarterContent,
    Video,
};

USTRUCT()
struct FLaunchPadPageActionData {
    GENERATED_BODY()

    UPROPERTY()
    ELaunchPadActionType Type;

    UPROPERTY()
    FString Path;

    UPROPERTY()
    FString Icon;

    UPROPERTY()
    FString Title;

    UPROPERTY()
    float Width = 0;
};

USTRUCT()
struct FLaunchPadPageLayout_Details {
    GENERATED_BODY()

    UPROPERTY()
    FString Title;

    UPROPERTY()
    FString Desc;

    UPROPERTY()
    FString Image;

    UPROPERTY()
    TArray<FLaunchPadPageActionData> Actions;

};

