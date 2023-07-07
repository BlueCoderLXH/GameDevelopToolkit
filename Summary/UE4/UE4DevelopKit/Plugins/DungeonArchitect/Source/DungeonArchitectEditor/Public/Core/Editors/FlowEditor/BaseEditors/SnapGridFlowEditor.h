//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/FlowEditor/FlowEditor.h"
#include "Core/Editors/FlowEditor/FlowEditorSettings.h"
#include "Core/Editors/FlowEditor/FlowTestRunner.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleDatabase.h"
#include "SnapGridFlowEditor.generated.h"

/////////////////// Snap Grid Flow Editor ///////////////////

UCLASS()
class USnapGridFlowEditorSettings : public UFlowEditorSettings {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Dungeon")
    TSoftObjectPtr<USnapGridFlowModuleDatabase> ModuleDatabase;
};

class FSnapGridFlowEditor : public FFlowEditorBase {
public:
    typedef FFlowEditorBase Super;
    
    // FFlowEditorBase
    virtual void InitDungeonConfig(UDungeonConfig* Config) override;
    virtual void CreateDomainEditors() override;
    virtual FName GetFlowEdAppName() const override;
    virtual ADungeon* CreatePreviewDungeon(UWorld* World) override;
    virtual FText GetEditorBrandingText() const override;
    virtual UFlowEditorSettings* CreateEditorSettingsObject() const override;
    virtual TSharedRef<SWidget> CreatePerfWidget(const TSharedRef<SDockTab> DockTab, TSharedPtr<SWindow> OwnerWindow) override;
    virtual void OnTestRunnerServiceStarted() override;
    virtual bool ShouldBuildPreviewDungeon() const override;
    // End of FFlowEditorBase

    // FAssetEditorToolkit
    virtual FName GetToolkitFName() const override;
    virtual FText GetBaseToolkitName() const override;
    virtual FString GetWorldCentricTabPrefix() const override;
    // End of FAssetEditorToolkit

protected:
    virtual bool RequiresCustomFrameLayout() const override { return true; }
    virtual TSharedPtr<FTabManager::FLayout> CreateFrameLayout() const override;
    virtual void  ConfigureDomainObject(IFlowDomainPtr Domain) override;

private:
    TSharedPtr<class SSnapGridFlowTestRunner> PerfRunner;
    
};

/////////////////// Snap Grid Flow Perf Editor ///////////////////
struct FSnapGridFlowTestRunnerSettings {
    int32 MaxTries = 0;
    TMap<FString, FString> ParameterOverrides;
    TWeakObjectPtr<UFlowAssetBase> FlowAsset;
    TWeakObjectPtr<class USnapGridFlowModuleDatabase> ModuleDatabase;
};

class FSnapGridFlowTestRunnerTask : public FFlowTestRunnerTaskBase<FSnapGridFlowTestRunnerSettings> {
public:
    virtual TSharedPtr<IFlowProcessDomainExtender> CreateDomainExtender(const FSnapGridFlowTestRunnerSettings& InSettings) override;
};


class SSnapGridFlowTestRunner : public SFlowTestRunnerBase<FSnapGridFlowTestRunnerTask, FSnapGridFlowTestRunnerSettings>
{
public:
    typedef SFlowTestRunnerBase<FSnapGridFlowTestRunnerTask, FSnapGridFlowTestRunnerSettings> Super;
    
    SLATE_BEGIN_ARGS(SSnapGridFlowTestRunner) { }
        SLATE_EVENT(FFlowTestRunnerServiceEvent, OnServiceStarted)
        SLATE_EVENT(FFlowTestRunnerServiceEvent, OnServiceStopped)
    SLATE_END_ARGS()


    void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow, TWeakObjectPtr<UFlowAssetBase> InFlowAsset);
    virtual void SetupSettingsObject(FSnapGridFlowTestRunnerSettings& OutSettings) override;

    void SetModuleDatabase(TWeakObjectPtr<USnapGridFlowModuleDatabase> InModuleDatabase);

private:
    TWeakObjectPtr<USnapGridFlowModuleDatabase> ModuleDatabase;
};



