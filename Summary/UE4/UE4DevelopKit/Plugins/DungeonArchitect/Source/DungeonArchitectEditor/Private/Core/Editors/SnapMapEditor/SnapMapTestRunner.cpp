//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/SnapMapTestRunner.h"

#include "Builders/SnapMap/SnapMapAsset.h"
#include "Frameworks/GraphGrammar/GraphGrammarProcessor.h"
#include "Frameworks/GraphGrammar/Script/GrammarScriptGraph.h"
#include "Frameworks/Snap/SnapMap/SnapMapLibrary.h"
#include "Frameworks/Snap/SnapMap/SnapMapModuleDatabase.h"

#define LOCTEXT_NAMESPACE "SSnapFlowPerfEditor"
DEFINE_LOG_CATEGORY_STATIC(LogSnapFlowTestRunner, Log, All);


//////////////////// Test Task //////////////////// 

void FSnapMapTestRunnerTask::Execute(const FSnapMapTestRunnerSettings& InSettings,
                                      FSnapMapTestRunnerStats& InOutStats) {
    USnapMapAsset* FlowAsset = InSettings.FlowAsset.Get();
    if (!FlowAsset) {
        return;
    }

    if (!InSettings.ModuleDatabase.IsValid()) {
        return;
    }
    
    FRandomStream RandomStream;
    RandomStream.GenerateNewSeed();

    const int32 MAX_RETRIES = FMath::Max(1, InSettings.MaxTries);
    int32 NumTries = 0;
    while (NumTries < MAX_RETRIES) {
        NumTries++;

        UGrammarScriptGraph* MissionGraph = NewObject<UGrammarScriptGraph>();
        UGraphGrammar* MissionGrammar = FlowAsset->MissionGrammar;
        int32 Seed = RandomStream.RandRange(0, MAX_int32 - 1);

        // build the mission graph from the mission grammar rules
        {
            FGraphGrammarProcessor GraphGrammarProcessor;
            GraphGrammarProcessor.Initialize(MissionGraph, MissionGrammar, Seed);
            GraphGrammarProcessor.Execute(MissionGraph, MissionGrammar);
        }

        SnapLib::FGrowthStaticState StaticState;
        StaticState.Random = RandomStream;
        StaticState.BoundsContraction = InSettings.CollisionTestContraction;
        StaticState.DungeonBaseTransform = FTransform::Identity;
        StaticState.StartTimeSecs = FPlatformTime::Seconds();
        StaticState.MaxProcessingTimeSecs = InSettings.MaxProcessingTimeSecs;
        StaticState.bAllowModuleRotations = InSettings.bAllowModuleRotations;

        SnapLib::IModuleDatabasePtr ModDB = MakeShareable(new FSnapMapModuleDatabaseImpl(InSettings.ModuleDatabase.Get()));
        SnapLib::FSnapGraphGenerator GraphGenerator(ModDB, StaticState);
        SnapLib::ISnapGraphNodePtr StartNode = MakeShareable(new FSnapGraphGrammarNode(MissionGraph->FindRootNode()));
        SnapLib::FModuleNodePtr RootNode = GraphGenerator.Generate(StartNode);
        bool bSuccess = RootNode.IsValid();
        if (bSuccess) break;
    }

    int32& Count = InOutStats.NumTriesCount.FindOrAdd(NumTries);
    Count++;

}

//////////////////// Test Runner Widget //////////////////// 

void SSnapMapTestRunner::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab,
                                    const TSharedPtr<SWindow>& ConstructUnderWindow,
                                    TWeakObjectPtr<USnapMapAsset> InFlowAsset) {
    OnServiceStarted = InArgs._OnServiceStarted;
    OnServiceStopped = InArgs._OnServiceStopped;

    FlowAsset = InFlowAsset;
    Settings = NewObject<USnapMapPerfEditorSettings>();
    Histogram = SNew(SDATestRunnerHistogram, SharedThis(this));

    SDATestRunner::Construct(SDATestRunner::FArguments(), ConstructUnderMajorTab, ConstructUnderWindow);
}


UObject* SSnapMapTestRunner::GetSettingsObject() {
    return Settings;
}

FText SSnapMapTestRunner::GetStatusText() const {
    FSnapMapTestRunnerStats Stats = TestRunner.GetStats();
    return FText::FromString(FString::Printf(
        TEXT("Pass [%d], Warn [%d], Fail [%d]"), HistogramData.NumPass, HistogramData.NumWarn, HistogramData.NumFail));
}

void SSnapMapTestRunner::StartService() {
    OnServiceStarted.ExecuteIfBound();
    
    Settings->WarningRetryThreshold = FMath::Min(Settings->WarningRetryThreshold, Settings->MaxRetries);
    Settings->PassRetryThreshold = FMath::Min(Settings->PassRetryThreshold, Settings->WarningRetryThreshold);

    Settings->MaxRetries = FMath::Max(1, Settings->MaxRetries);
    Settings->PassRetryThreshold = FMath::Max(1, Settings->PassRetryThreshold);
    Settings->WarningRetryThreshold = FMath::Max(1, Settings->WarningRetryThreshold);

    FSnapMapTestRunnerSettings TestSettings;
    TestSettings.MaxTries = Settings->MaxRetries;
    TestSettings.ModuleDatabase = Settings->ModuleDatabase;
    TestSettings.FlowAsset = FlowAsset;
    TestSettings.bAllowModuleRotations = Settings->bAllowModuleRotations;
    TestSettings.MaxProcessingTimeSecs = Settings->MaxProcessingTimeSecs;
    TestSettings.CollisionTestContraction = Settings->CollisionTestContraction;

    Settings->GarbageCollectEveryNTests = FMath::Max(1, Settings->GarbageCollectEveryNTests);
    GarbageCollectedAtTestNumber = 0;
    RunGC();

    TestRunner.StartService(Settings->NumTests, TestSettings);
}

void SSnapMapTestRunner::StopService() {
    TestRunner.StopService();

    RunGC();

    OnServiceStopped.ExecuteIfBound();
}

void SSnapMapTestRunner::TickService() {
    TestRunner.Tick();
    BuildHistogramData();

    if (GarbageCollectedAtTestNumber + Settings->GarbageCollectEveryNTests < TestRunner.GetCompletedTasks()) {
        RunGC();
        GarbageCollectedAtTestNumber = TestRunner.GetCompletedTasks();
    }
}

bool SSnapMapTestRunner::IsServiceRunning() const {
    return TestRunner.IsRunning();
}

void SSnapMapTestRunner::NotifyTestsComplete() {
    SDATestRunner::NotifyTestsComplete();

    RunGC();
}

TSharedRef<SWidget> SSnapMapTestRunner::GetHistogramWidget() const {
    const TSharedRef<SWidget> HistogramWidget = Histogram.IsValid()
                                                    ? Histogram.ToSharedRef()
                                                    : SNullWidget::NullWidget;

    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Text(LOCTEXT("HistogramLabel", "X = Num Retries, Y = Num Test Cases"))
        ]
        + SVerticalBox::Slot()
        [
            HistogramWidget
        ];

}

void SSnapMapTestRunner::AddReferencedObjects(FReferenceCollector& Collector) {
    if (Settings) {
        Collector.AddReferencedObject(Settings);
    }
}

bool SSnapMapTestRunner::ValidateConfiguration(FText& OutErrorMessage) {
    if (!Settings->ModuleDatabase) {
        OutErrorMessage = LOCTEXT("ErrorNoModule", "Missing Module Database reference in settings");
        return false;
    }
    return true;
}

int32 SSnapMapTestRunner::GetNumBars() {
    return HistogramData.BarValues.Num();
}

float SSnapMapTestRunner::GetTotalValue() {
    return HistogramData.MaxValue;
}

float SSnapMapTestRunner::GetMinValue() {
    return 0;
}

float SSnapMapTestRunner::GetBarValue(int32 Index) {
    return Index < HistogramData.BarValues.Num() ? HistogramData.BarValues[Index] : 0;
}

FString SSnapMapTestRunner::GetBarText(int32 Index) {
    return Index < HistogramData.BarTexts.Num() ? HistogramData.BarTexts[Index] : "";
}

FLinearColor SSnapMapTestRunner::GetBarColor(int32 Index) {
    return Index < HistogramData.BarColors.Num() ? HistogramData.BarColors[Index] : FLinearColor::White;
}

void SSnapMapTestRunner::RunGC() {
    // Collect garbage to clear out the destroyed level
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

void SSnapMapTestRunner::BuildHistogramData() {
    FSnapMapTestRunnerStats Stats = TestRunner.GetStats();
    int32 NumBars = FMath::Min(8, Settings->MaxRetries);
    HistogramData = FSnapMapTestRunnerHistogramData();
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

#undef LOCTEXT_NAMESPACE

