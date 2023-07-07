//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

template <typename TSettings, typename TStats>
class IDATestRunner {
public:
    virtual ~IDATestRunner() {
    }

    virtual void Tick() {
    }

    virtual void StartService(int32 InNumTasks, const TSettings& InSettings) = 0;
    virtual void StopService() = 0;
    virtual TStats GetStats() const = 0;
    virtual bool IsRunning() const = 0;
    virtual int32 GetCompletedTasks() const = 0;
    virtual int32 GetTotalTasks() const = 0;
};

