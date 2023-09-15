//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

/** Notification class for asynchronous dungeon building. */
class FDungeonBuildingNotification // : public FTickableEditorObject
{

public:

    FDungeonBuildingNotification() {
    }

    virtual ~FDungeonBuildingNotification() {
    }

    void SetBuildingInProgress(bool bBuildingInProgress);
protected:

    /** Starts the notification. */
    void BuildStarted();

    /** Ends the notification. */
    void BuildFinished();

    /** FTickableEditorObject interface */
    virtual void Tick(float DeltaTime);

    virtual bool IsTickable() const {
        return true;
    }

    //virtual TStatId GetStatId() const override;

private:
    void ClearCompleteNotification();
    FText GetNotificationText() const;

    TWeakPtr<SNotificationItem> DungeonBuildNotificationPtr;

    bool bBuildingInProgress;

};

