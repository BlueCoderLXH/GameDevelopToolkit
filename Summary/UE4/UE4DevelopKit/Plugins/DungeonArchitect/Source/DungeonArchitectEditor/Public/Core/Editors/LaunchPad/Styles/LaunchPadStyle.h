//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"
#include "LaunchPadStyle.generated.h"

USTRUCT()
struct FDALaunchPadStyleRegistryPath {
    GENERATED_BODY()

    UPROPERTY()
    FString Id;

    UPROPERTY()
    FString Path;

    UPROPERTY()
    int32 Width;

    UPROPERTY()
    int32 Height;
};

USTRUCT()
struct FDALaunchPadStyleRegistry {
    GENERATED_BODY()

    UPROPERTY()
    TArray<FDALaunchPadStyleRegistryPath> Paths;
};

class FDALaunchPadStyle {
public:
    static void Initialize();

    static void Shutdown();

    static const ISlateStyle& Get();

    static FName GetStyleSetName();

    class FStyle : public FSlateStyleSet {
    public:
        FStyle();
        void Initialize();

    private:
        FTextBlockStyle NormalText;
    };

private:
    static TSharedRef<class FSlateStyleSet> Create();

private:
    static TSharedPtr<class FSlateStyleSet> StyleInstance;
};

