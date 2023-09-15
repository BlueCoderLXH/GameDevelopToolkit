//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "IPropertyChangeListener.h"
#include "DungeonArchitectEditorCustomization.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDungeonCustomization, Log, All);

class FDungeonArchitectEditorCustomization : public IDetailCustomization {
public:
    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    // End of IDetailCustomization interface

    static TSharedRef<IDetailCustomization> MakeInstance();

    static FReply RebuildDungeon(IDetailLayoutBuilder* DetailBuilder);
    static FReply DestroyDungeon(IDetailLayoutBuilder* DetailBuilder);
    static FReply RandomizeSeed(IDetailLayoutBuilder* DetailBuilder);
    static FReply OpenHelpSystem(IDetailLayoutBuilder* DetailBuilder);
};

class FSnapModuleDatabaseCustomization : public IDetailCustomization {
public:
    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    // End of IDetailCustomization interface

    static TSharedRef<IDetailCustomization> MakeInstance();
    static FReply BuildDatabaseCache(IDetailLayoutBuilder* DetailBuilder);
};

UCLASS()
class USnapGridFlowModuleDBImportSettings : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = Module)
    FName Category = "Room";

    UPROPERTY(EditAnywhere, Category = Module)
    bool bAllowRotation = true;
};

class FSnapGridFlowModuleDatabaseCustomization : public IDetailCustomization, public FGCObject {
public:
    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    // End of IDetailCustomization interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

    static TSharedRef<IDetailCustomization> MakeInstance();
    static FReply BuildDatabaseCache(IDetailLayoutBuilder* DetailBuilder);
    
    FReply HandleTool_AddModulesFromDir(IDetailLayoutBuilder* DetailBuilder);
    FReply HandleTool_AddModulesFromDir_ButtonOK(USnapGridFlowModuleDBImportSettings* InSettings, class USnapGridFlowModuleDatabase* ModuleDatabase, FString ImportPath);
    FReply HandleTool_AddModulesFromDir_ButtonCancel();

private:
    USnapGridFlowModuleDBImportSettings* DirImportSettings = nullptr;
    TWeakPtr<SWindow> DirImportSettingsWindow;
};

class FDungeonArchitectMeshNodeCustomization : public IDetailCustomization {
public:
    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    // End of IDetailCustomization interface

    static FReply EditAdvancedOptions(IDetailLayoutBuilder* DetailBuilder);
};


class FDungeonEditorViewportPropertiesCustomization : public IDetailCustomization {
public:
    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    // End of IDetailCustomization interface

    static TSharedRef<IDetailCustomization> MakeInstance();
};

class FDungeonArchitectVolumeCustomization : public IDetailCustomization {
public:
    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    // End of IDetailCustomization interface

    static TSharedRef<IDetailCustomization> MakeInstance();

    static FReply RebuildDungeon(IDetailLayoutBuilder* DetailBuilder);
};

class FDAExecRuleNodeCustomization : public IDetailCustomization {
public:
    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;
    // End of IDetailCustomization interface

    void OnExecutionModeChanged(class UEdGraphNode_ExecRuleNode* Node);

    static TSharedRef<IDetailCustomization> MakeInstance();
private:
    TWeakPtr<IDetailLayoutBuilder> CachedDetailBuilder;
};

class FDungeonPropertyChangeListener : public TSharedFromThis<FDungeonPropertyChangeListener> {
public:
    void Initialize();
    void OnPropertyChanged(UObject* Object, struct FPropertyChangedEvent& Event);

private:
    TSharedPtr<IPropertyChangeListener> PropertyChangeListener;
};


class FDungeonDebugCustomization : public IDetailCustomization {
public:
    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    // End of IDetailCustomization interface

    static TSharedRef<IDetailCustomization> MakeInstance();

    static FReply ExecuteCommand(IDetailLayoutBuilder* DetailBuilder, int32 CommandID);
};

