#include "FrameTaskMgr.h"

DEFINE_LOG_CATEGORY(LogFrameTask);

#define FRAME_TASK_STAT(FuncName, DebugName) \
	const FString StatDebugName = FString::Printf(TEXT("%s_[Callback]%s"), TEXT(FuncName), *DebugName); \
	const TStatId StatId = FDynamicStats::CreateStatId<FStatGroup_STATGROUP_UObjects>( StatDebugName ); \
	FScopeCycleCounter CycleCounter( StatId );

bool FFrameTask::Tick()
{
#if	STATS
	FRAME_TASK_STAT("FFrameTask::Tick", DebugFuncName);
#endif

	if (!ShouldRun()) return false;

	return Run();
}

void FFrameTask::Unbind()
{
    NativeEvent.Unbind();
    UFuncEvent.Unbind();
}

void FFrameTaskQueue::Tick()
{
#if	STATS
	FRAME_TASK_STAT("FFrameTaskQueue::Tick", Name.ToString());
#endif

	if (IsEmpty()) return;

	const int32 NextIndex = TaskQueue.Num() - 1;
	
	FFrameTask& NextTask = TaskQueue[NextIndex];
	if (NextTask.Tick())
	{
		TaskQueue.RemoveAt(NextIndex);
	}
}

void FFrameTaskQueue::Clear()
{
    for (FFrameTask& Task : TaskQueue)
    {
        Task.Unbind();
    }
    
    TaskQueue.Empty();
}


FFrameTaskQueue& FFrameTaskMgr::FindOrAddQueue(const FName& InQueueName)
{
	FFrameTaskQueue* PTaskQueue = TaskQueues.Find(InQueueName);
	if (!PTaskQueue)
	{
		PTaskQueue = &TaskQueues.Add(InQueueName, FFrameTaskQueue(InQueueName));
	}

	return *PTaskQueue;
}

bool FFrameTaskMgr::IsTickable() const
{
	return TaskQueues.Num() > 0;
}

void FFrameTaskMgr::Tick(float DeltaTime)
{
	static TArray<FName> ToBeRemoved;
	
	if (TaskQueues.Num() <= 0) return;
	
	for (TTuple<FName, FFrameTaskQueue>& PairItem : TaskQueues) 
	{
		FFrameTaskQueue& TaskQueue = PairItem.Value;
		TaskQueue.Tick();

		if (TaskQueue.IsEmpty())
		{
			ToBeRemoved.Add(PairItem.Key);
		}
	}

	for (const FName& RemovedKey : ToBeRemoved)
	{
		TaskQueues.Remove(RemovedKey);
	}

	ToBeRemoved.Reset();
}

void UFrameTaskLib::AddFrameTask(const FName QueueName, const FFrameTaskUFuncEvent& TaskEvent, int32 SkipFrames/* = 1*/)
{
	SkipFrames = FMath::Max(SkipFrames, 1);
	FFrameTaskMgr::Get().AddFrameTask(QueueName, TaskEvent, SkipFrames);
}

void UFrameTaskLib::RemoveFrameQueue(const FName QueueName)
{
    FFrameTaskMgr::Get().RemoveQueue(QueueName);
}
