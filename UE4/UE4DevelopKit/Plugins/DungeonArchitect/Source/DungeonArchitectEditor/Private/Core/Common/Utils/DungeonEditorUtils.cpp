//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Common/Utils/DungeonEditorUtils.h"

#include "Core/Dungeon.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "EditorActorFolders.h"
#include "EditorViewportClient.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IAssetViewport.h"

#define LOCTEXT_NAMESPACE "DungeonEditorUtils"
DEFINE_LOG_CATEGORY(LogDungeonEditorUtils);

ADungeon* FDungeonEditorUtils::GetDungeonActorFromLevelViewport() {
    FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
    TSharedPtr<IAssetViewport> ViewportWindow = LevelEditorModule.GetFirstActiveViewport();
    ADungeon* DungeonCandidate = nullptr;
    if (ViewportWindow.IsValid()) {
        FEditorViewportClient& Viewport = ViewportWindow->GetAssetViewportClient();
        UWorld* World = Viewport.GetWorld();
        for (TActorIterator<ADungeon> DungeonIt(World); DungeonIt; ++DungeonIt) {
            ADungeon* Dungeon = *DungeonIt;
            if (Dungeon->IsSelected()) {
                return Dungeon;
            }
            DungeonCandidate = Dungeon;
        }
    }
    return DungeonCandidate;
}

void FDungeonEditorUtils::ShowNotification(
    FText Text, SNotificationItem::ECompletionState State /*= SNotificationItem::CS_Fail*/) {
    FNotificationInfo Info(Text);
    Info.bFireAndForget = true;
    Info.FadeOutDuration = 1.0f;
    Info.ExpireDuration = 2.0f;

    TWeakPtr<SNotificationItem> NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
    if (NotificationPtr.IsValid()) {
        NotificationPtr.Pin()->SetCompletionState(State);
    }
}

void FDungeonEditorUtils::SwitchToRealtimeMode() {
    FEditorViewportClient* client = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
    if (client) {
        bool bRealtime = client->IsRealtime();
        if (!bRealtime) {
            ShowNotification(
                NSLOCTEXT("DungeonRealtimeMode", "DungeonRealtimeMode", "Switched viewport to Realtime mode"),
                SNotificationItem::CS_None);
            client->SetRealtime(true);
        }
    }
    else {
        ShowNotification(NSLOCTEXT("ClientNotFound", "ClientNotFound", "Warning: Cannot find active viewport"));
    }
}

void FDungeonEditorUtils::CreateDungeonItemFolder(ADungeon* Dungeon) {
    if (Dungeon && FActorFolders::IsAvailable()) {
        UWorld& World = *Dungeon->GetWorld();
        const FScopedTransaction Transaction(LOCTEXT("UndoAction_CreateFolder", "Create Folder"));
        const FName NewFolderName = FActorFolders::Get().GetDefaultFolderNameForSelection(World);

        auto& Folders = FActorFolders::Get();
        FString FullPath = Dungeon->GetName() + "_Items";
        FName Path(*FullPath);

        Folders.CreateFolder(World, Path);
        Dungeon->ItemFolderPath = Path;
    }
    else {
        // Folder manager does not exist.  Clear the folder path from the dungeon actor,
        // so they are spawned in the root folder node
        Dungeon->ItemFolderPath = FName();
    }
}

void FDungeonEditorUtils::CollapseDungeonItemFolder(ADungeon* Dungeon) {
    if (Dungeon && Dungeon->GetWorld() && FActorFolders::IsAvailable()) {
        FActorFolderProps* FolderProperties = FActorFolders::Get().GetFolderProperties(
            *Dungeon->GetWorld(), Dungeon->ItemFolderPath);
        if (FolderProperties) {
            FolderProperties->bIsExpanded = false;
        }
    }
}

void FDungeonEditorUtils::BuildDungeon(ADungeon* Dungeon) {
    SwitchToRealtimeMode();
    CreateDungeonItemFolder(Dungeon);
    Dungeon->BuildDungeon();
    CollapseDungeonItemFolder(Dungeon);
}

FAssetPackageInfo FDungeonAssetUtils::DuplicateAsset(UObject* SourceAsset, const FString& TargetPackageName,
                                                     const FString& TargetObjectName) {
    FAssetPackageInfo Duplicate;
    if (SourceAsset) {
        // Make sure the referenced object is deselected before duplicating it.
        GEditor->GetSelectedObjects()->Deselect(SourceAsset);

        // Duplicate the asset
        Duplicate.Package = CreatePackage(*TargetPackageName);
        Duplicate.Asset = StaticDuplicateObject(SourceAsset, Duplicate.Package, *TargetObjectName);

        if (Duplicate.Asset) {
            Duplicate.Asset->MarkPackageDirty();

            // Notify the asset registry
            FAssetRegistryModule::AssetCreated(Duplicate.Asset);
        }
        else {
            UE_LOG(LogDungeonEditorUtils, Error, TEXT("Failed to duplicate asset %s"), *TargetObjectName);
        }
    }

    return Duplicate;
}

void FDungeonAssetUtils::SaveAsset(const FAssetPackageInfo& Info) {
    if (Info.Asset && Info.Package) {
        Info.Package->SetDirtyFlag(true);
        FString PackagePath = Info.Package->GetOutermost()->GetName();
        //const FString PackagePath = FString::Printf(TEXT("%s/%s_Copy"), *GetGamePath(), *AssetName);
        if (!UPackage::SavePackage(Info.Package, nullptr, RF_Standalone,
                                   *FPackageName::LongPackageNameToFilename(
                                       PackagePath, FPackageName::GetAssetPackageExtension()), GError, nullptr, false,
                                   true, SAVE_NoError)) {
            UE_LOG(LogDungeonEditorUtils, Display, TEXT("Unable to save asset %s"), *Info.Asset->GetName());
        }
    }
}


#undef LOCTEXT_NAMESPACE

