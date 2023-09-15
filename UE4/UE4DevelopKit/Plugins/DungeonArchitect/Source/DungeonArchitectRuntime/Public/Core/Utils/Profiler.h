//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Misc/DateTime.h"

struct Profiler {
    Profiler() : elapsedTime(0) {
    }

    FDateTime frameStartTime, frameEndTime;
    double elapsedTime;
};

#define PROFILE_DECLARE(ProfilerName) \
	Profiler ProfilerName;

#define PROFILE_START(ProfilerName) \
	ProfilerName.frameStartTime = FDateTime::Now();

#define PROFILE_STOP(ProfilerName) \
	ProfilerName.frameEndTime = FDateTime::Now();		\
	ProfilerName.elapsedTime += (ProfilerName.frameEndTime - ProfilerName.frameStartTime).GetTotalMilliseconds();

#define PROFILE_RESET(ProfilerName) \
	ProfilerName.elapsedTime = 0;

#define PROFILE_PRINT(ProfilerName, LogCategory, Title) \
	UE_LOG(LogCategory, Warning, TEXT("%s: %f"), TEXT(Title), ProfilerName.elapsedTime);

#define PROFILE_FUNCTION(LogCategory, Title, Statement) \
{	\
	FDateTime frameStartTime = FDateTime::Now();	\
	Statement;										\
	FDateTime frameEndTime = FDateTime::Now();		\
	FTimespan frameTime = frameEndTime - frameStartTime;	\
	UE_LOG(LogCategory, Warning, TEXT("%s: %f"), TEXT(Title), frameTime.GetTotalMilliseconds());	\
}

