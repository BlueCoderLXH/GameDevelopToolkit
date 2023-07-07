//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/FlowEditor/FlowEditor.h"
#include "Core/Editors/FlowEditor/FlowTestRunner.h"

//////////////////// Grid Flow Editor ////////////////////

class FGridFlowEditor : public FFlowEditorBase {
public:
    // FFlowEditorBase
    virtual void InitDungeonConfig(UDungeonConfig* Config) override;
    virtual void CreateDomainEditors() override;
    virtual FName GetFlowEdAppName() const override;
    virtual ADungeon* CreatePreviewDungeon(UWorld* World) override;
    virtual FText GetEditorBrandingText() const override;
    virtual TSharedRef<SWidget> CreatePerfWidget(const TSharedRef<SDockTab> DockTab, TSharedPtr<SWindow> OwnerWindow) override;
    // End of FFlowEditorBase

    // FAssetEditorToolkit
    virtual FName GetToolkitFName() const override;
    virtual FText GetBaseToolkitName() const override;
    virtual FString GetWorldCentricTabPrefix() const override;
    // End of FAssetEditorToolkit
};

//////////////////// Grid Flow Perf Editor ////////////////////
struct FGridFlowTestRunnerSettings {
    int32 MaxTries = 0;
    TMap<FString, FString> ParameterOverrides;
    TWeakObjectPtr<UFlowAssetBase> FlowAsset;
};

class FGridFlowTestRunnerTask : public FFlowTestRunnerTaskBase<FGridFlowTestRunnerSettings> {
public:
    virtual TSharedPtr<IFlowProcessDomainExtender> CreateDomainExtender(const FGridFlowTestRunnerSettings& InSettings) override;
};

class SGridFlowTestRunner : public SFlowTestRunnerBase<FGridFlowTestRunnerTask, FGridFlowTestRunnerSettings>
{
public:
    typedef SFlowTestRunnerBase<FGridFlowTestRunnerTask, FGridFlowTestRunnerSettings> Super;
    
    SLATE_BEGIN_ARGS(SGridFlowTestRunner) { }
        SLATE_EVENT(FFlowTestRunnerServiceEvent, OnServiceStarted)
        SLATE_EVENT(FFlowTestRunnerServiceEvent, OnServiceStopped)
    SLATE_END_ARGS()


    void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab,
               const TSharedPtr<SWindow>& ConstructUnderWindow, TWeakObjectPtr<UFlowAssetBase> InFlowAsset);
};

