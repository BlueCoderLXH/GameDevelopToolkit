//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/TestRunner/DATestRunner.h"

#include "HAL/ThreadSafeBool.h"

template <typename TTask, typename TSettings, typename TStats>
class TDATestRunnerAuxThread
    : public FRunnable
      , public IDATestRunner<TSettings, TStats> {
public:
    virtual ~TDATestRunnerAuxThread() {
        StopService();
    }

    virtual void StartService(int32 InNumTasks, const TSettings& InSettings) override {
        StopService();

        check(Thread == nullptr);
        NumTasks = InNumTasks;
        Settings = InSettings;
        Stats = TStats();
        NumCompletedTasks = 0;

        Thread = FRunnableThread::Create(this, TEXT("Test Runner Thread"), 0, TPri_BelowNormal);
    }

    virtual void StopService() override {
        if (Thread) {
            Thread->Kill();
            delete Thread;
            Thread = nullptr;
        }
    }

    virtual TStats GetStats() const override {
        FScopeLock Lock(&TaskExecMutex);
        return Stats;
    }

    virtual bool IsRunning() const override { return bIsRunning; }
    virtual int32 GetCompletedTasks() const override { return NumCompletedTasks; }
    virtual int32 GetTotalTasks() const override { return NumTasks; }

protected:
    virtual void Stop() override {
        bRequestStop = true;
    }

    virtual bool Init() override {
        if (bIsRunning) {
            return false;
        }

        bRequestStop = false;
        return true;
    }

    virtual uint32 Run() override {
        bIsRunning = true;

        NumCompletedTasks = 0;
        while (!bRequestStop && NumCompletedTasks < NumTasks) {
            FScopeLock Lock(&TaskExecMutex);
            TTask Task;
            Task.Execute(Settings, Stats);
            NumCompletedTasks++;
        }

        bIsRunning = false;
        return 0;
    }

protected:
    FRunnableThread* Thread = nullptr;
    FThreadSafeCounter NumCompletedTasks;
    FThreadSafeBool bIsRunning;
    FThreadSafeBool bRequestStop;
    FCriticalSection TaskExecMutex;

    int32 NumTasks = 0;
    TSettings Settings;
    TStats Stats;
};

