//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Common/Utils/DungeonBuildingNotification.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void FDungeonBuildingNotification::BuildStarted() {
    FNotificationInfo Info(GetNotificationText());
    Info.bFireAndForget = false;
    Info.FadeOutDuration = 0.0f;
    Info.ExpireDuration = 0.0f;

    DungeonBuildNotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
    if (DungeonBuildNotificationPtr.IsValid()) {
        DungeonBuildNotificationPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
    }
}

void FDungeonBuildingNotification::BuildFinished() {
    TSharedPtr<SNotificationItem> NotificationItem = DungeonBuildNotificationPtr.Pin();
    if (NotificationItem.IsValid()) {
        NotificationItem->SetText(NSLOCTEXT("DungeonBuild", "DungeonBuildingComplete", "Dungeon Rebuilt"));
        NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
        NotificationItem->ExpireAndFadeout();
        DungeonBuildNotificationPtr.Reset();
    }
}

void FDungeonBuildingNotification::Tick(float DeltaTime) {

}

//TStatId FDungeonBuildingNotification::GetStatId() const
//{
//	RETURN_QUICK_DECLARE_CYCLE_STAT(FNavigationBuildingNotificationImpl, STATGROUP_Tickables);
//}

void FDungeonBuildingNotification::ClearCompleteNotification() {

}

FText FDungeonBuildingNotification::GetNotificationText() const {
    return FText(NSLOCTEXT("DungeonBuild", "DungeonBuildingInProgress", "Building Dungeon"));
}

void FDungeonBuildingNotification::SetBuildingInProgress(bool bCurrentBuildInProgress) {
    if (this->bBuildingInProgress == bCurrentBuildInProgress) return;
    this->bBuildingInProgress = bCurrentBuildInProgress;

    if (bBuildingInProgress) {
        BuildStarted();
    }
    else {
        BuildFinished();
    }
}

