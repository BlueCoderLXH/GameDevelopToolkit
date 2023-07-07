//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/FlowAsset.h"
#include "Frameworks/Flow/FlowProcessor.h"
#include "Frameworks/TestRunner/DATestRunnerGameThread.h"
#include "Frameworks/TestRunner/Widgets/SDATestRunner.h"
#include "Frameworks/TestRunner/Widgets/SDATestRunnerHistogram.h"

#include "UObject/GCObject.h"
#include "FlowTestRunner.generated.h"

class UFlowAssetBase;
class SDockTab;
class SWindow;
class FTabManager;
class FMenuBuilder;
class SDATestRunnerHistogram;

///////////////////// Flow Test Runner Framework ///////////////////// 

UCLASS()
class UFlowPerfEditorSettings : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Settings")
    int32 NumTests = 300;

    UPROPERTY(EditAnywhere, Category = "Settings")
    int32 MaxRetries = 100;

    UPROPERTY(EditAnywhere, Category = "Settings")
    int32 PassRetryThreshold = 50;

    UPROPERTY(EditAnywhere, Category = "Settings")
    int32 WarningRetryThreshold = 80;

    UPROPERTY(EditAnywhere, Category = "Settings")
    TMap<FString, FString> ParameterOverrides;

    /** 
     Have the editor free up resources after every N tests
     If you run a total of 2000 tests, and the specified value is 100,
     the memory will be freed up at test number 100, 200, 300, and so on
    */
    UPROPERTY(EditAnywhere, Category = "Advanced")
    int32 GarbageCollectEveryNTests = 100;
};


struct FFlowTestRunnerStats {
    TMap<int32, int32> NumTriesCount; // NumTriesForBuild, TestCount
};

template<typename TSettings>
class FFlowTestRunnerTaskBase {
public:
    virtual ~FFlowTestRunnerTaskBase() {}
    void Execute(const TSettings& InSettings, FFlowTestRunnerStats& InOutStats) {
        UFlowAssetBase* FlowAsset = InSettings.FlowAsset.Get();
        if (!FlowAsset) {
            return;
        }

        FRandomStream RandomStream;
        RandomStream.GenerateNewSeed();

        FFlowProcessor FlowProcessor;
        // Register domains to the flow processor
        {
            TSharedPtr<IFlowProcessDomainExtender> DomainExtender = CreateDomainExtender(InSettings);
            check(DomainExtender.IsValid());
            DomainExtender->ExtendDomains(FlowProcessor);
        }
    
        const int32 MAX_RETRIES = FMath::Max(1, InSettings.MaxTries);
        int32 NumTries = 0;
        int32 NumTimeouts = 0;
        while (NumTries < MAX_RETRIES) {
            NumTries++;
            
            FFlowProcessorSettings ProcessorSettings;
            ProcessorSettings.SerializedAttributeList = InSettings.ParameterOverrides;

            const FFlowProcessorResult Result = FlowProcessor.Process(FlowAsset->ExecScript, RandomStream, ProcessorSettings);
            if (Result.ExecResult == EFlowTaskExecutionResult::Success) {
                break;
            }
            if (Result.ExecResult == EFlowTaskExecutionResult::FailHalt) {
                NumTries = MAX_RETRIES;
                break;
            }
        }
        int32& Count = InOutStats.NumTriesCount.FindOrAdd(NumTries);
        Count++;
    }
    virtual TSharedPtr<IFlowProcessDomainExtender> CreateDomainExtender(const TSettings& InSettings) = 0;
};

///////////////////// Flow Test Runner Widget ///////////////////// 
struct FFlowTestRunnerHistogramData {
    TArray<int32> BarValues;
    TArray<FString> BarTexts;
    TArray<FLinearColor> BarColors;
    int32 MinValue = 0;
    int32 MaxValue = 0;

    int32 NumPass = 0;
    int32 NumWarn = 0;
    int32 NumFail = 0;
};


DECLARE_DELEGATE(FFlowTestRunnerServiceEvent);

template<typename TTask, typename TSettings>
class SFlowTestRunnerBase
    : public SDATestRunner
     , public FGCObject
     , public IDATestRunnerHistogramDataSource {
public:
    
    SLATE_BEGIN_ARGS(SFlowTestRunnerBase) { }
        SLATE_EVENT(FFlowTestRunnerServiceEvent, OnServiceStarted)
        SLATE_EVENT(FFlowTestRunnerServiceEvent, OnServiceStopped)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab,
               const TSharedPtr<SWindow>& ConstructUnderWindow, TWeakObjectPtr<UFlowAssetBase> InFlowAsset) {
        OnServiceStarted = InArgs._OnServiceStarted;
        OnServiceStopped = InArgs._OnServiceStopped;

        FlowAsset = InFlowAsset;
        Settings = NewObject<UFlowPerfEditorSettings>();
        Histogram = SNew(SDATestRunnerHistogram, SharedThis(this));

        SDATestRunner::Construct(SDATestRunner::FArguments(), ConstructUnderMajorTab, ConstructUnderWindow);
    }
    

    virtual UObject* GetSettingsObject() override { return Settings; }
    virtual FText GetStatusText() const override {
        FFlowTestRunnerStats Stats = TestRunner.GetStats();
        return FText::FromString(FString::Printf(
            TEXT("Pass [%d], Warn [%d], Fail [%d]"), HistogramData.NumPass, HistogramData.NumWarn, HistogramData.NumFail));
    }

    virtual void StartService() override {
        OnServiceStarted.ExecuteIfBound();

        Settings->WarningRetryThreshold = FMath::Min(Settings->WarningRetryThreshold, Settings->MaxRetries);
        Settings->PassRetryThreshold = FMath::Min(Settings->PassRetryThreshold, Settings->WarningRetryThreshold);

        Settings->MaxRetries = FMath::Max(1, Settings->MaxRetries);
        Settings->PassRetryThreshold = FMath::Max(1, Settings->PassRetryThreshold);
        Settings->WarningRetryThreshold = FMath::Max(1, Settings->WarningRetryThreshold);

        TSettings TestSettings;
        SetupSettingsObject(TestSettings);

        Settings->GarbageCollectEveryNTests = FMath::Max(1, Settings->GarbageCollectEveryNTests);
        GarbageCollectedAtTestNumber = 0;
        RunGC();

        TestRunner.StartService(Settings->NumTests, TestSettings);
    }
    
    virtual void StopService() override {
        TestRunner.StopService();
        RunGC();
        OnServiceStopped.ExecuteIfBound();
    }

    virtual void SetupSettingsObject(TSettings& OutSettings) {
        OutSettings.MaxTries = Settings->MaxRetries;
        OutSettings.ParameterOverrides = Settings->ParameterOverrides;
        OutSettings.FlowAsset = FlowAsset;
    } 
    
    virtual void TickService() override {
        TestRunner.Tick();
        BuildHistogramData();

        if (GarbageCollectedAtTestNumber + Settings->GarbageCollectEveryNTests < TestRunner.GetCompletedTasks()) {
            RunGC();
            GarbageCollectedAtTestNumber = TestRunner.GetCompletedTasks();
        }
    }
    
    virtual bool IsServiceRunning() const override {
        return TestRunner.IsRunning();
    }
    
    virtual void NotifyTestsComplete() override {
        SDATestRunner::NotifyTestsComplete();

        RunGC();
    }
    
    virtual TSharedRef<SWidget> GetHistogramWidget() const override {
        const TSharedRef<SWidget> HistogramWidget = Histogram.IsValid()
                                                        ? Histogram.ToSharedRef()
                                                        : SNullWidget::NullWidget;

        return SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(STextBlock)
                .Justification(ETextJustify::Center)
                .Text(NSLOCTEXT("FlowTestRunner", "HistogramLabel", "X = Num Retries, Y = Num Test Cases"))
            ]
            + SVerticalBox::Slot()
            [
                HistogramWidget
            ];
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override {
        if (Settings) {
            Collector.AddReferencedObject(Settings);
        }
    }


    //~ Begin IDATestRunnerHistogramDataSource Interface
    virtual int32 GetNumBars() override { return HistogramData.BarValues.Num(); }
    virtual float GetTotalValue() override { return HistogramData.MaxValue; }
    virtual float GetMinValue() override { return 0; }
    virtual float GetBarValue(int32 Index) override {
        return Index < HistogramData.BarValues.Num() ? HistogramData.BarValues[Index] : 0;
    }
    virtual FString GetBarText(int32 Index) override {
        return Index < HistogramData.BarTexts.Num() ? HistogramData.BarTexts[Index] : "";
    }
    virtual FLinearColor GetBarColor(int32 Index) override {
        return Index < HistogramData.BarColors.Num() ? HistogramData.BarColors[Index] : FLinearColor::White;
    }
    //~ End IDATestRunnerHistogramDataSource Interface

private:
    void RunGC() {
        // Collect garbage to clear out the destroyed level
        CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    }
    void BuildHistogramData() {
        FFlowTestRunnerStats Stats = TestRunner.GetStats();
        int32 NumBars = FMath::Min(8, Settings->MaxRetries);
        HistogramData = FFlowTestRunnerHistogramData();
        if (Stats.NumTriesCount.Num() > 0) {
            for (auto& Entry : Stats.NumTriesCount) {
                int32 NumTries = Entry.Key;
                int32 Count = Entry.Value;

                NumBars = FMath::Max(NumBars, NumTries);
                if (NumTries <= Settings->PassRetryThreshold) {
                    HistogramData.NumPass += Count;
                }
                else if (NumTries <= Settings->WarningRetryThreshold) {
                    HistogramData.NumWarn += Count;
                }
                else {
                    HistogramData.NumFail += Count;
                }

            }
        }

        for (int i = 0; i < NumBars; i++) {
            int32 NumTries = i + 1;

            // Set the Bar Value
            int32& BarValue = Stats.NumTriesCount.FindOrAdd(NumTries);
            HistogramData.BarValues.Add(BarValue);

            // Set the Bar Text
            FString BarText = FString::FromInt(NumTries);
            if (NumTries == Settings->MaxRetries) {
                BarText += "+";
            }
            HistogramData.BarTexts.Add(BarText);

            // Set the Bar Color
            FLinearColor BarColor;
            if (NumTries <= Settings->PassRetryThreshold) {
                BarColor = FLinearColor::Green;
            }
            else if (NumTries <= Settings->WarningRetryThreshold) {
                BarColor = FLinearColor(1.0f, 0.25f, 0.0f, 1.0f);
            }
            else {
                BarColor = FLinearColor::Red;
            }
            HistogramData.BarColors.Add(BarColor);
            HistogramData.MaxValue = FMath::Max(HistogramData.MaxValue, BarValue);
        }
    }

protected:
    TWeakObjectPtr<UFlowAssetBase> FlowAsset;
    TSharedPtr<SDATestRunnerHistogram> Histogram;

    UFlowPerfEditorSettings* Settings = nullptr;
    //TTestRunner TestRunner;
    TDATestRunnerGameThread<TTask, TSettings, FFlowTestRunnerStats> TestRunner;
    int32 GarbageCollectedAtTestNumber = 0;
    FFlowTestRunnerHistogramData HistogramData;

    FFlowTestRunnerServiceEvent OnServiceStarted;
    FFlowTestRunnerServiceEvent OnServiceStopped;
};

