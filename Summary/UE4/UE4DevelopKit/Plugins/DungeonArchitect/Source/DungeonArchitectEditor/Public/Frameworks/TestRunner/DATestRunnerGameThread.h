//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/TestRunner/DATestRunner.h"

template <typename TTask, typename TSettings, typename TStats>
class TDATestRunnerGameThread
    : public IDATestRunner<TSettings, TStats> {
public:
    TDATestRunnerGameThread(float InTimePerFrameInSeconds = 0.05f) {
        TimePerFrameInSeconds = InTimePerFrameInSeconds;
    }

    virtual void StartService(int32 InNumTasks, const TSettings& InSettings) override {
        NumTasks = InNumTasks;
        Settings = InSettings;
        Stats = TStats();
        NumCompletedTasks = 0;
        bIsRunning = true;
    }

    virtual void StopService() override {
        bIsRunning = false;
    }

    virtual TStats GetStats() const override { return Stats; }
    virtual bool IsRunning() const override { return bIsRunning; }
    virtual int32 GetCompletedTasks() const override { return NumCompletedTasks; }
    virtual int32 GetTotalTasks() const override { return NumTasks; }

    virtual void Tick() override {
        if (NumCompletedTasks >= NumTasks) {
            bIsRunning = false;
        }

        if (bIsRunning) {
            const double StartTime = FPlatformTime::Seconds();
            double CurrentTime = StartTime;
            while (bIsRunning && NumCompletedTasks < NumTasks && CurrentTime - StartTime <= TimePerFrameInSeconds) {
                TTask Task;
                Task.Execute(Settings, Stats);
                NumCompletedTasks++;
                CurrentTime = FPlatformTime::Seconds();
            }

            if (NumCompletedTasks >= NumTasks) {
                bIsRunning = false;
            }
        }
    }

protected:
    float TimePerFrameInSeconds;
    int32 NumCompletedTasks = 0;
    bool bIsRunning = false;

    int32 NumTasks = 0;
    TSettings Settings;
    TStats Stats;
};

