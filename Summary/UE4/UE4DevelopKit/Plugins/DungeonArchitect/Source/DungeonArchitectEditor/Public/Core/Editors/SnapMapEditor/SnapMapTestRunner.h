//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/TestRunner/DATestRunnerGameThread.h"
#include "Frameworks/TestRunner/Widgets/SDATestRunner.h"
#include "Frameworks/TestRunner/Widgets/SDATestRunnerHistogram.h"
#include "SnapMapTestRunner.generated.h"

///////////////////// Snap Flow Test Runner Framework ///////////////////// 

class USnapMapAsset;
class USnapMapModuleDatabase;
class USnapMapDungeonConfig;
UCLASS()
class USnapMapPerfEditorSettings : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Test Settings")
    int32 NumTests = 300;

    UPROPERTY(EditAnywhere, Category = "Test Settings")
    int32 MaxRetries = 10;

    UPROPERTY(EditAnywhere, Category = "Test Settings")
    int32 PassRetryThreshold = 5;

    UPROPERTY(EditAnywhere, Category = "Test Settings")
    int32 WarningRetryThreshold = 8;

    /** 
    Have the editor free up resources after every N tests
    If you run a total of 2000 tests, and the specified value is 100,
    the memory will be freed up at test number 100, 200, 300, and so on
    */
    UPROPERTY(EditAnywhere, Category = "Test Settings")
    int32 GarbageCollectEveryNTests = 100;

    UPROPERTY(EditAnywhere, Category = "Snap Config")
    USnapMapModuleDatabase* ModuleDatabase = nullptr;
    
    UPROPERTY(EditAnywhere, Category = "Snap Config")
    bool bAllowModuleRotations = true;
    
    UPROPERTY(EditAnywhere, Category = "Snap Config")
    int32 CollisionTestContraction = 100;

    /** The max time allotted per test before it is considered a failure */
    UPROPERTY(EditAnywhere, Category = "Snap Config")
    float MaxProcessingTimeSecs = 3.0f;
};


struct FSnapMapTestRunnerSettings {
    int32 MaxTries;
    TWeakObjectPtr<USnapMapModuleDatabase> ModuleDatabase;
    TWeakObjectPtr<USnapMapAsset> FlowAsset;
    float MaxProcessingTimeSecs;
    bool bAllowModuleRotations;
    int32 CollisionTestContraction;
};

struct FSnapMapTestRunnerStats {
    TMap<int32, int32> NumTriesCount; // NumTriesForBuild, TestCount
};

class FSnapMapTestRunnerTask {
public:
    void Execute(const FSnapMapTestRunnerSettings& InSettings, FSnapMapTestRunnerStats& InOutStats);
};

///////////////////// Snap Flow Test Runner Widget ///////////////////// 
struct FSnapMapTestRunnerHistogramData {
    TArray<int32> BarValues;
    TArray<FString> BarTexts;
    TArray<FLinearColor> BarColors;
    int32 MinValue = 0;
    int32 MaxValue = 0;

    int32 NumPass = 0;
    int32 NumWarn = 0;
    int32 NumFail = 0;
};

typedef TDATestRunnerGameThread<FSnapMapTestRunnerTask, FSnapMapTestRunnerSettings, FSnapMapTestRunnerStats> FSnapMapTestRunner;

DECLARE_DELEGATE(FSnapMapTestRunnerServiceEvent);


class SSnapMapTestRunner
    : public SDATestRunner
     , public FGCObject
     , public IDATestRunnerHistogramDataSource
{
public:
    
    SLATE_BEGIN_ARGS(SSnapMapTestRunner) { }
        SLATE_EVENT(FSnapMapTestRunnerServiceEvent, OnServiceStarted)
        SLATE_EVENT(FSnapMapTestRunnerServiceEvent, OnServiceStopped)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab,
                   const TSharedPtr<SWindow>& ConstructUnderWindow, TWeakObjectPtr<USnapMapAsset> FlowAsset);

    virtual UObject* GetSettingsObject() override;
    virtual FText GetStatusText() const override;

    virtual void StartService() override;
    virtual void StopService() override;
    virtual bool IsServiceRunning() const override;
    virtual void TickService() override;
    virtual void NotifyTestsComplete() override;
    virtual TSharedRef<SWidget> GetHistogramWidget() const override;

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual bool ValidateConfiguration(FText& OutErrorMessage) override;

    //~ Begin IDATestRunnerHistogramDataSource Interface
    virtual int32 GetNumBars() override;
    virtual float GetTotalValue() override;
    virtual float GetMinValue() override;
    virtual float GetBarValue(int32 Index) override;
    virtual FString GetBarText(int32 Index) override;
    virtual FLinearColor GetBarColor(int32 Index) override;
    //~ End IDATestRunnerHistogramDataSource Interface

private:
    void RunGC();
    void BuildHistogramData();

private:
    TWeakObjectPtr<USnapMapAsset> FlowAsset;
    TSharedPtr<SDATestRunnerHistogram> Histogram;

    USnapMapPerfEditorSettings* Settings = nullptr;
    FSnapMapTestRunner TestRunner;
    int32 GarbageCollectedAtTestNumber = 0;
    FSnapMapTestRunnerHistogramData HistogramData;

    FSnapMapTestRunnerServiceEvent OnServiceStarted;
    FSnapMapTestRunnerServiceEvent OnServiceStopped;
};



