#pragma once

#include "CoreMinimal.h"
#include "FrameTaskMgr.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFrameTask, Log, All);

DECLARE_DELEGATE(FFrameTaskNativeEvent);
DECLARE_DYNAMIC_DELEGATE(FFrameTaskUFuncEvent);

/**
 * 分帧Task
 */
struct FFrameTask final
{
private:
	int32 WaitFrames = 0;
	FFrameTaskNativeEvent NativeEvent;
	FFrameTaskUFuncEvent UFuncEvent;
	
#if STATS
	FString DebugFuncName;
#endif	

	bool ShouldRun() { return --WaitFrames <= 0; }
	bool Run() const
	{
		if (NativeEvent.IsBound())
		{
			return NativeEvent.ExecuteIfBound();
		}
		return UFuncEvent.ExecuteIfBound();
	}	

public:
	FFrameTask() { }

	explicit FFrameTask(const FFrameTaskNativeEvent& InNativeEvent, const int32 InWaitFrames = 1)
	{
		NativeEvent = InNativeEvent;
		WaitFrames = InWaitFrames;

#if STATS && USE_DELEGATE_TRYGETBOUNDFUNCTIONNAME
		DebugFuncName = NativeEvent.TryGetBoundFunctionName().ToString();
#endif		
	}
	
	explicit FFrameTask(const FFrameTaskUFuncEvent& InUFuncEvent, const int32 InWaitFrames = 1)
	{
		UFuncEvent = InUFuncEvent;
		WaitFrames = InWaitFrames;

#if STATS
		DebugFuncName = UFuncEvent.GetFunctionName().ToString();
#endif		
	}
	
	void WaitForNextFrames(const int32 InWaitFrames = 1) { check(InWaitFrames > 0); WaitFrames = InWaitFrames; }

	bool Tick();

    void Unbind();
};

/**
 * 分帧Task队列
 */
struct FFrameTaskQueue final
{
private:
	static constexpr uint8 Default_Size = 16;
	
	FName Name;
	TArray<FFrameTask> TaskQueue;
	
public:
	FFrameTaskQueue()
	{
		TaskQueue.Reserve(Default_Size);
	}

	explicit FFrameTaskQueue(const FName& InQueueName)
	{		
		Name = InQueueName;
		TaskQueue.Reserve(Default_Size);
	}

	template<typename CallbackType>
	FFrameTask& AddFrameTask(const CallbackType& InLambda, const uint32 InSkipFrames = 1)
	{
		TaskQueue.EmplaceAt(0, FFrameTask(FFrameTaskNativeEvent::CreateLambda(InLambda), InSkipFrames));
		return TaskQueue[0];
	}
	
	FFrameTask& AddFrameTask(const FFrameTaskUFuncEvent& InUFuncEvent, const uint32 InSkipFrames = 1)
	{
		TaskQueue.EmplaceAt(0, FFrameTask(InUFuncEvent, InSkipFrames));
		return TaskQueue[0];
	}

	bool IsEmpty() const { return TaskQueue.Num() <= 0; }

	void Tick();

    void Clear();

	friend struct FFrameTaskMgr;
};

/**
 * 分帧Task管理
 */
struct FFrameTaskMgr final : public FTickableGameObject
{
private:
	typedef FName FQueueName;
	
	TMap<FQueueName, FFrameTaskQueue> TaskQueues;

	FFrameTaskMgr() {}

	static FFrameTaskMgr& Get()
	{
		static FFrameTaskMgr FrameTaskMgr;
		return FrameTaskMgr;
	}
	
	FFrameTaskQueue& FindOrAddQueue(const FQueueName& InQueueName);
	
	static FFrameTaskQueue& GetQueue(const FQueueName& InQueueName)
	{
		FFrameTaskMgr& TaskMgr = Get();
		return TaskMgr.FindOrAddQueue(InQueueName);
	}

	template<typename CallbackType>
	static FFrameTask& AddFrameTask(const FQueueName& InQueueName, CallbackType&& InLambda, const uint32 InSkipFrames = 1)
	{
		return GetQueue(InQueueName).AddFrameTask(InLambda, InSkipFrames);
	}
	
	static FFrameTask& AddFrameTask(const FQueueName& InQueueName, const FFrameTaskUFuncEvent& InUFuncEvent, const uint32 InSkipFrames = 1)
	{
		return GetQueue(InQueueName).AddFrameTask(InUFuncEvent, InSkipFrames);
	}

    static void RemoveQueue(const FQueueName& InQueueName)
	{
	    FFrameTaskMgr& TaskMgr = Get();
	    FFrameTaskQueue* PTaskQueue = TaskMgr.TaskQueues.Find(InQueueName);
	    if (PTaskQueue)
	    {
	        PTaskQueue->Clear();
	        TaskMgr.TaskQueues.Remove(InQueueName);
	    }
	}
	
public:

	virtual bool IsTickable() const override;

	virtual void Tick(float DeltaTime) override;
	
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(STAT_FFRAMETASKMGR_TICK, STATGROUP_Tickables);
	}

	friend class UFrameTaskLib;
};

/**
 * 分帧Task蓝图库函数
 */
UCLASS(meta=(ScriptName="FrameTaskMgr", ToolTip="Create multi frame tasks for optimizing cpu usage"))
class GAME_API UFrameTaskLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    /**
     * 向名为'QueueName'的队列中增加分帧任务（BP）
     * @param QueueName 分帧队列的名字
     * @param TaskEvent 新增分帧任务的事件回调
     * @param SkipFrames 新增分帧任务间隔的帧数(相比队列中前一个分帧任务)
     */
    UFUNCTION(BlueprintCallable, meta=(ToolTip="Add frame task: 'SkipFrames' should be no less than 1!"))
	static void AddFrameTask(const FName QueueName, UPARAM(ref) const FFrameTaskUFuncEvent& TaskEvent, int32 SkipFrames = 1);

    /**
     * 向名为'QueueName'的队列中增加分帧任务（C++）
     * @param InQueueName 分帧队列的名字
     * @param InLambda 新增分帧任务的Lambda回调
     * @param InSkipFrames 新增分帧任务间隔的帧数(相比队列中前一个分帧任务)
     * @return 新增分帧任务的对象
     */
    template<typename CallbackType>
	static FFrameTask& AddFrameTask(const FName& InQueueName, CallbackType&& InLambda, int32 InSkipFrames = 1)
	{
		InSkipFrames = FMath::Max(InSkipFrames, 1);
		return FFrameTaskMgr::Get().AddFrameTask(InQueueName, InLambda, InSkipFrames);
	}

    /**
     * RemoveFrameTaskQueue
     * 打断并清空名为'QueueName'的分帧队列（BP/C++）
     * @param QueueName 分帧队列的名字
     */
    UFUNCTION(BlueprintCallable)
    static void RemoveFrameQueue(const FName QueueName);    
};
