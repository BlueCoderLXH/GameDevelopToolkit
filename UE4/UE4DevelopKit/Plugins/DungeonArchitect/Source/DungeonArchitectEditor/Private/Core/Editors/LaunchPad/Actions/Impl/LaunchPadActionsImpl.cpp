//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Actions/Impl/LaunchPadActionsImpl.h"

#include "Builders/GridFlow/GridFlowAsset.h"
#include "Builders/GridFlow/GridFlowConfig.h"
#include "Builders/SnapGridFlow/SnapGridFlowAsset.h"
#include "Builders/SnapGridFlow/SnapGridFlowDungeon.h"
#include "Builders/SnapMap/SnapMapAsset.h"
#include "Builders/SnapMap/SnapMapDungeonConfig.h"
#include "Core/Common/Utils/DungeonEditorUtils.h"
#include "Core/Dungeon.h"
#include "Core/Editors/FlowEditor/BaseEditors/GridFlowEditor.h"
#include "Core/Editors/FlowEditor/BaseEditors/SnapGridFlowEditor.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadData.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditor.h"
#include "Core/Editors/ThemeEditor/DungeonArchitectThemeEditor.h"
#include "Frameworks/Snap/SnapMap/SnapMapModuleDatabase.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "IAddContentDialogModule.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "ObjectTools.h"
#include "Widgets/SWidget.h"

#define LOCTEXT_NAMESPACE "FLaunchPadActions"
DEFINE_LOG_CATEGORY_STATIC(LogLaunchPadActionsImpl, Log, All);

void FLaunchPadActions::Execute(const FLaunchPadPageActionData& Data, TSharedPtr<SWidget> HostWidget) {
    switch (Data.Type) {
    case ELaunchPadActionType::OpenFolder:
        Exec_OpenFolder(Data.Path);
        break;

    case ELaunchPadActionType::OpenScene:
        Exec_OpenScene(Data.Path);
        break;

    case ELaunchPadActionType::OpenTheme:
        Exec_OpenTheme(Data.Path);
        break;

    case ELaunchPadActionType::OpenSnapFlow:
        Exec_OpenSnapFlow(Data.Path);
        break;

    case ELaunchPadActionType::OpenGridFlow:
        Exec_OpenGridFlow(Data.Path);
        break;

    case ELaunchPadActionType::CloneScene:
        Exec_CloneScene(Data.Path);
        break;

    case ELaunchPadActionType::CloneSceneAndBuild:
        Exec_CloneSceneAndBuild(Data.Path);
        break;

    case ELaunchPadActionType::CloneTheme:
        Exec_CloneTheme(Data.Path);
        break;

    case ELaunchPadActionType::CloneSnapFlow:
        Exec_CloneSnapFlow(Data.Path);
        break;

    case ELaunchPadActionType::CloneGridFlow:
        Exec_CloneGridFlow(Data.Path);
        break;

    case ELaunchPadActionType::CloneSnapGridFlow:
        Exec_CloneSnapGridFlow(Data.Path);
        break;
            
    case ELaunchPadActionType::Documentation:
        Exec_Documentation(Data.Path);
        break;

    case ELaunchPadActionType::AddStarterContent:
        Exec_AddStarterContent(HostWidget);
        break;

    case ELaunchPadActionType::Video:
        Exec_Video(Data.Path);
        break;

    case ELaunchPadActionType::LauncherURL:
        Exec_LauncherURL(Data.Path);
        break;

    case ELaunchPadActionType::None:
    default:
        break;
    }
}

namespace {
    UWorld* GetEditorWorld() {
        TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);

        if (!IsInGameThread() || !GIsEditor) {
            return nullptr;
        }
        if (GEditor->PlayWorld || GIsPlayInEditorWorld) {
            return nullptr;
        }

        return GEditor ? GEditor->GetEditorWorldContext(false).World() : nullptr;
    }

    template <typename T>
    T* LoadObjectFromPath(const FString& Path) {
        T* Object = FindObject<T>(nullptr, *Path);
        if (!Object) {
            Object = LoadObject<T>(nullptr, *Path);
        }
        return Object;
    }

    template <typename TAssetClass>
    bool ShowSaveDialog(const FString& InDefaultPath, FString& OutSavePath) {
        FString PackageRoot;
        FString PackagePath;
        FString PackageName;
        FPackageName::SplitLongPackageName(InDefaultPath, PackageRoot, PackagePath, PackageName);

        FSaveAssetDialogConfig SaveAssetDialogConfig;
        {
            SaveAssetDialogConfig.DefaultAssetName = PackageName;
            SaveAssetDialogConfig.AssetClassNames.Add(TAssetClass::StaticClass()->GetFName());
            SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;
            SaveAssetDialogConfig.DialogTitleOverride = LOCTEXT("SaveLevelDialogTitle", "Clone Asset As");
        }

        FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
            "ContentBrowser");
        IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();
        OutSavePath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
        if (OutSavePath.IsEmpty()) {
            return false;
        }
        return true;
    }

    template <typename TAssetClass>
    bool CloneAsset(TAssetClass* SourceAsset, const FString& SaveObjectPath, TAssetClass*& OutClonedAsset,
                    FString& OutTargetPackageName) {
        OutTargetPackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
        FString NewPackageRoot;
        FString NewPackagePath;
        FString NewObjectName;
        FPackageName::SplitLongPackageName(OutTargetPackageName, NewPackageRoot, NewPackagePath, NewObjectName);

        ObjectTools::FPackageGroupName PGN;
        PGN.PackageName = OutTargetPackageName;
        PGN.ObjectName = NewObjectName;

        TSet<UPackage*> LockedPackages;
        UObject* ClonedAsset = ObjectTools::DuplicateSingleObject(SourceAsset, PGN, LockedPackages);
        OutClonedAsset = Cast<TAssetClass>(ClonedAsset);
        return (OutClonedAsset != nullptr);

    }

    template <typename TAssetClass>
    bool CloneAsset(TAssetClass* SourceAsset, const FString& SaveObjectPath, TAssetClass*& OutClonedAsset) {
        FString TargetPackageName;
        return CloneAsset(SourceAsset, SaveObjectPath, OutClonedAsset, TargetPackageName);
    }

    void ShowAssetInContentBrowser(UObject* Object) {
        IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
            "ContentBrowser").Get();
        ContentBrowser.SyncBrowserToAssets({FAssetData(Object)});
    }

    template <typename T>
    T* CloneWithDialog() {
        return nullptr;
    }

    template <typename TAsset, typename TAssetEditor>
    bool CloneEditableAsset(const FString& InSourcePath,
                            TFunction<void(TSharedRef<TAssetEditor>, TAsset*)> FuncInitEditor) {
        TAsset* SourceAsset = LoadObjectFromPath<TAsset>(InSourcePath);
        if (!SourceAsset) {
            return false;
        }

        FString SaveObjectPath;
        if (ShowSaveDialog<TAsset>(InSourcePath, SaveObjectPath)) {
            FString TargetPackageName;
            TAsset* ClonedAsset;
            if (CloneAsset(SourceAsset, SaveObjectPath, ClonedAsset, TargetPackageName)) {
                // Load the theme editor with this theme
                TSharedRef<TAssetEditor> AssetEditor(new TAssetEditor());
                FuncInitEditor(AssetEditor, ClonedAsset);

                // Navigate to the folder
                ShowAssetInContentBrowser(ClonedAsset);

                return true;
            }
        }

        return false;
    }

    void CreateUniqueAssetName(const FString& InPackageName, const FString& InAssetName, FString& OutPackageName,
                               FString& OutAssetName) {
        FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
        AssetToolsModule.Get().CreateUniqueAssetName(InPackageName, InAssetName, OutPackageName, OutAssetName);
    }

    FString CreateUniqueAssetName(const FString& InPackageName, const FString& InAssetName) {
        FString OutPackageName, OutAssetName;
        CreateUniqueAssetName(InPackageName, InAssetName, OutPackageName, OutAssetName);
        return OutPackageName;
    }
}

void FLaunchPadActions::Exec_OpenFolder(const FString& InPath) {
    IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
    ContentBrowser.ForceShowPluginContent(true);

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    const FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(*InPath);
    ContentBrowser.SyncBrowserToAssets({AssetData});
}

bool FLaunchPadActions::Exec_OpenScene(const FString& InPath) {
    // If there are any unsaved changes to the current level, see if the user wants to save those first.
    bool bPromptUserToSave = true;
    bool bSaveMapPackages = true;
    bool bSaveContentPackages = true;
    if (FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages)) {
        if (FPackageName::IsValidLongPackageName(*InPath)) {
            const FString FileToOpen = FPackageName::LongPackageNameToFilename(
                *InPath, FPackageName::GetMapPackageExtension());
            const bool bLoadAsTemplate = false;
            const bool bShowProgress = true;
            FEditorFileUtils::LoadMap(FileToOpen, bLoadAsTemplate, bShowProgress);
            return true;
        }
    }
    return false;
}

bool FLaunchPadActions::Exec_OpenTheme(const FString& InPath) {
    UDungeonThemeAsset* ThemeAsset = LoadObjectFromPath<UDungeonThemeAsset>(InPath);
    if (ThemeAsset) {
        TSharedRef<FDungeonArchitectThemeEditor> NewDungeonEditor(new FDungeonArchitectThemeEditor());
        NewDungeonEditor->InitDungeonEditor(EToolkitMode::Standalone, nullptr, ThemeAsset);
        return true;
    }
    return false;
}

bool FLaunchPadActions::Exec_OpenSnapFlow(const FString& InPath) {
    USnapMapAsset* Asset = LoadObjectFromPath<USnapMapAsset>(InPath);
    if (Asset) {
        TSharedRef<FSnapMapEditor> NewDungeonEditor(new FSnapMapEditor());
        NewDungeonEditor->InitFlowEditor(EToolkitMode::Standalone, nullptr, Asset);
        return true;
    }
    return false;
}

bool FLaunchPadActions::Exec_OpenGridFlow(const FString& InPath) {
    UGridFlowAsset* Asset = LoadObjectFromPath<UGridFlowAsset>(InPath);
    if (Asset) {
        TSharedRef<FGridFlowEditor> NewDungeonEditor(new FGridFlowEditor());
        NewDungeonEditor->InitEditor(EToolkitMode::Standalone, nullptr, Asset);
        return true;
    }
    return false;
}

bool FLaunchPadActions::Exec_OpenSnapGridFlow(const FString& InPath, USnapGridFlowModuleDatabase* InModuleDatabase) {
    USnapGridFlowAsset* Asset = LoadObjectFromPath<USnapGridFlowAsset>(InPath);
    if (Asset) {
        TSharedRef<FSnapGridFlowEditor> NewEditor(new FSnapGridFlowEditor());
        NewEditor->InitEditor(EToolkitMode::Standalone, nullptr, Asset);
        if (USnapGridFlowEditorSettings* EditorSettings = Cast<USnapGridFlowEditorSettings>(NewEditor->GetEditorSettings())) {
            EditorSettings->ModuleDatabase = InModuleDatabase;
        }
        return true;
    }
    return false;
}

bool FLaunchPadActions::Exec_CloneScene(const FString& InPath) {
    if (!FPackageName::IsValidLongPackageName(InPath)) {
        UE_LOG(LogLaunchPadActionsImpl, Log, TEXT("Cannot clone. Invalid source package name"));
        return false;
    }

    bool bPromptUserToSave = true;
    bool bSaveMapPackages = true;
    bool bSaveContentPackages = true;
    // If there are any unsaved changes to the current level, see if the user wants to save those first.
    if (!FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages)) {
        // The current level has pending changes and the user doesn't want to abandon it yet
        return false;
    }

    UWorld* SourceAsset = LoadObjectFromPath<UWorld>(InPath);
    if (!SourceAsset) {
        return false;
    }

    FString SaveObjectPath;
    if (ShowSaveDialog<UWorld>(InPath, SaveObjectPath)) {
        FString TargetPackageName;
        UWorld* ClonedAsset;
        if (CloneAsset(SourceAsset, SaveObjectPath, ClonedAsset, TargetPackageName)) {
            // Load the level
            const FString FileToOpen = FPackageName::LongPackageNameToFilename(
                TargetPackageName, FPackageName::GetMapPackageExtension());
            const bool bLoadAsTemplate = false;
            const bool bShowProgress = true;
            FEditorFileUtils::LoadMap(FileToOpen, bLoadAsTemplate, bShowProgress);

            // Clone the referenced items
            UWorld* World = GetEditorWorld();
            if (World) {
                FString PackageRoot, PackagePath, LevelObjectName;
                FPackageName::SplitLongPackageName(TargetPackageName, PackageRoot, PackagePath, LevelObjectName);
                FString TargetDirectory = PackageRoot + PackagePath;
                for (TActorIterator<ADungeon> It(World); It; ++It) {
                    ADungeon* Dungeon = *It;
                    if (Dungeon) {
                        // Clone the referenced themes
                        {
                            TArray<UDungeonThemeAsset*> ClonedThemes;
                            for (UDungeonThemeAsset* SourceTheme : Dungeon->Themes) {
                                FString ClonePackageName = CreateUniqueAssetName(
                                    TargetDirectory, SourceTheme->GetName());
                                UDungeonThemeAsset* ClonedTheme = nullptr;
                                if (CloneAsset(SourceTheme, ClonePackageName, ClonedTheme)) {
                                    ClonedThemes.Add(ClonedTheme);
                                }
                                else {
                                    ClonedThemes.Add(SourceTheme);
                                }
                            }
                            Dungeon->Themes = ClonedThemes;

                            /*
                            // Open the theme editor
                            for (UDungeonThemeAsset* Theme : Dungeon->Themes) {
                                if (!Theme) continue;
                                Exec_OpenTheme(Theme->GetPathName());
                                break;
                            }
                            */
                        }

                        // Clone the referenced grid flow graph
                        if (UGridFlowConfig* GridFlowConfig = Cast<UGridFlowConfig>(Dungeon->GetConfig())) {
                            UGridFlowAsset* SourceGridFlowAsset = GridFlowConfig->GridFlow.LoadSynchronous();
                            if (SourceGridFlowAsset) {
                                FString ClonePackageName = CreateUniqueAssetName(
                                    TargetDirectory, SourceGridFlowAsset->GetName());
                                UGridFlowAsset* ClonedGridFlowAsset = nullptr;
                                if (CloneAsset(SourceGridFlowAsset, ClonePackageName, ClonedGridFlowAsset)) {
                                    GridFlowConfig->GridFlow = ClonedGridFlowAsset;
                                }

                                // Open the grid flow editor
                                UGridFlowAsset* AssetToOpen = GridFlowConfig->GridFlow.LoadSynchronous();
                                if (AssetToOpen) {
                                    Exec_OpenGridFlow(AssetToOpen->GetPathName());
                                }
                            }
                            GridFlowConfig->Modify();
                        }

                        // Clone the referenced snap flow graph
                        if (USnapMapDungeonConfig* SnapConfig = Cast<USnapMapDungeonConfig>(Dungeon->GetConfig())) {
                            // Clone the snap graph grammar asset
                            {
                                USnapMapAsset* SourceFlowAsset = SnapConfig->DungeonFlowGraph;
                                if (SourceFlowAsset) {
                                    FString ClonePackageName = CreateUniqueAssetName(
                                        TargetDirectory, SourceFlowAsset->GetName());
                                    USnapMapAsset* ClonedFlowAsset = nullptr;
                                    if (CloneAsset(SourceFlowAsset, ClonePackageName, ClonedFlowAsset)) {
                                        SnapConfig->DungeonFlowGraph = ClonedFlowAsset;
                                    }

                                    // Open the snap flow editor
                                    if (SnapConfig->DungeonFlowGraph) {
                                        Exec_OpenSnapFlow(SnapConfig->DungeonFlowGraph->GetPathName());
                                    }
                                }
                            }


                            // Clone the module database asset
                            {
                                USnapMapModuleDatabase* SourceModuleDB = SnapConfig->ModuleDatabase;
                                if (SourceModuleDB) {
                                    FString ClonePackageName = CreateUniqueAssetName(
                                        TargetDirectory, SourceModuleDB->GetName());
                                    USnapMapModuleDatabase* ClonedModuleDB = nullptr;
                                    if (CloneAsset(SourceModuleDB, ClonePackageName, ClonedModuleDB)) {
                                        SnapConfig->ModuleDatabase = ClonedModuleDB;
                                    }
                                }
                            }
                            SnapConfig->Modify();
                        }

                        
                        // Clone the referenced snap grid flow graph
                        if (USnapGridFlowConfig* SnapGridFlowConfig = Cast<USnapGridFlowConfig>(Dungeon->GetConfig())) {
                            
                            // Clone the module database asset
                            USnapGridFlowModuleDatabase* ClonedModuleDB = nullptr;
                            {
                                USnapGridFlowModuleDatabase* SourceModuleDB = SnapGridFlowConfig->ModuleDatabase.LoadSynchronous();
                                if (SourceModuleDB) {
                                    FString ClonePackageName = CreateUniqueAssetName(TargetDirectory, SourceModuleDB->GetName());
                                    if (CloneAsset(SourceModuleDB, ClonePackageName, ClonedModuleDB)) {
                                        SnapGridFlowConfig->ModuleDatabase = ClonedModuleDB;
                                    }
                                }
                            }
                            
                            // Clone the item theme file
                            {
                                UDungeonThemeAsset* SourceTheme = SnapGridFlowConfig->ItemTheme.LoadSynchronous();
                                if (SourceTheme) {
                                    FString ClonePackageName = CreateUniqueAssetName(TargetDirectory, SourceTheme->GetName());
                                    UDungeonThemeAsset* ClonedTheme = nullptr;
                                    if (CloneAsset(SourceTheme, ClonePackageName, ClonedTheme)) {
                                        SnapGridFlowConfig->ItemTheme = ClonedTheme;
                                    }
                                }
                            }

                            // Clone the Flow asset
                            {
                                USnapGridFlowAsset* SourceFlowAsset = SnapGridFlowConfig->FlowGraph.LoadSynchronous();
                                if (SourceFlowAsset) {
                                    FString ClonePackageName = CreateUniqueAssetName(TargetDirectory, SourceFlowAsset->GetName());
                                    USnapGridFlowAsset* ClonedGridFlowAsset = nullptr;
                                    if (CloneAsset(SourceFlowAsset, ClonePackageName, ClonedGridFlowAsset)) {
                                        SnapGridFlowConfig->FlowGraph = ClonedGridFlowAsset;
                                    }

                                    // Open the snap grid flow editor
                                    USnapGridFlowAsset* AssetToOpen = SnapGridFlowConfig->FlowGraph.LoadSynchronous();
                                    if (AssetToOpen) {
                                        Exec_OpenSnapGridFlow(AssetToOpen->GetPathName(), ClonedModuleDB);
                                    }
                                }
                            }

                            SnapGridFlowConfig->Modify();
                        }
                        
                        
                        Dungeon->Modify();
                    }
                }
            }

            return true;
        }
    }

    return false;
}

bool FLaunchPadActions::Exec_CloneSceneAndBuild(const FString& InPath) {
    if (!Exec_CloneScene(InPath)) {
        return false;
    }

    // Build all dungeons in the scene
    UWorld* World = GetEditorWorld();
    if (World) {
        for (TActorIterator<ADungeon> It(World); It; ++It) {
            ADungeon* Dungeon = *It;
            if (Dungeon) {
                FDungeonEditorUtils::BuildDungeon(Dungeon);
            }
        }
    }
    return true;
}

bool FLaunchPadActions::Exec_CloneTheme(const FString& InPath) {
    return CloneEditableAsset<UDungeonThemeAsset, FDungeonArchitectThemeEditor>(
        InPath, [](auto Editor, auto ClonedAsset) {
            Editor->InitDungeonEditor(EToolkitMode::Standalone, nullptr, ClonedAsset);
        });
}

bool FLaunchPadActions::Exec_CloneSnapFlow(const FString& InPath) {
    return CloneEditableAsset<USnapMapAsset, FSnapMapEditor>(InPath, [](auto Editor, auto ClonedAsset) {
        Editor->InitFlowEditor(EToolkitMode::Standalone, nullptr, ClonedAsset);
    });
}

bool FLaunchPadActions::Exec_CloneGridFlow(const FString& InPath) {
    return CloneEditableAsset<UGridFlowAsset, FGridFlowEditor>(InPath, [](auto Editor, auto ClonedAsset) {
        Editor->InitEditor(EToolkitMode::Standalone, nullptr, ClonedAsset);
    });
}

bool FLaunchPadActions::Exec_CloneSnapGridFlow(const FString& InPath) {
    return CloneEditableAsset<USnapGridFlowAsset, FSnapGridFlowEditor>(InPath, [](auto Editor, auto ClonedAsset) {
        Editor->InitEditor(EToolkitMode::Standalone, nullptr, ClonedAsset);
    });
}

void FLaunchPadActions::Exec_Documentation(const FString& InPath) {
    Exec_OpenURL(InPath);
}

void FLaunchPadActions::Exec_Video(const FString& InPath) {
    Exec_OpenURL(InPath);
}

void FLaunchPadActions::Exec_LauncherURL(const FString& InPath) {
    Exec_OpenURL(InPath);
}

void FLaunchPadActions::Exec_AddStarterContent(TSharedPtr<SWidget> HostWidget) {
    IAddContentDialogModule& AddContentDialogModule = FModuleManager::LoadModuleChecked<IAddContentDialogModule>("AddContentDialog");
    FWidgetPath WidgetPath;
    FSlateApplication::Get().GeneratePathToWidgetChecked(HostWidget.ToSharedRef(), WidgetPath);
    AddContentDialogModule.ShowDialog(WidgetPath.GetWindow());
}

void FLaunchPadActions::Exec_OpenURL(const FString& URL) {
    FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
}


#undef LOCTEXT_NAMESPACE

