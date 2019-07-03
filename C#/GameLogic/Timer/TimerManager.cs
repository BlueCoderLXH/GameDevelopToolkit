using System.Collections.Generic;
using UnityEngine.Events;

/// <summary>
/// 计时器管理
/// 
/// 计时器的创建/缓存/更新/删除
/// </summary>
public class SolarTimerManager
{
    protected class TimerPool : ManagedPool<SolarTimer> { }

    private static SolarTimerManager m_Instance;
    public static SolarTimerManager Instance
    {
        get
        {
            if (m_Instance == null)
            {
                m_Instance = new SolarTimerManager();
            }

            return m_Instance;
        }
    }

    // Timer的默认数量
    const int TimerDefaultSize = 10;

    // 默认Timer ID的起点
    const int DefaultTimerID = 10000;

    // 无效的TimerID
    const int InvalidTimerID = 0;

    // 标记递增的Timer ID
    static int IncreaseTimerID = DefaultTimerID;


    // Timer缓存池
    protected TimerPool m_TimerPool;

    // 保存需要增加的Timer对象(延时处理, 保证可以嵌套增加Timer)
    List<SolarTimer> m_TimersToBeAdded;

    // 保存需要删除的Timer对象(延时处理, 保证可以嵌套删除Timer)
    List<SolarTimer> m_TimersToBeRemoved;

    // Timer对象管理
    Dictionary<int, SolarTimer> m_Timers;

    private SolarTimerManager()
    {
        m_TimerPool = new TimerPool();

        m_Timers = new Dictionary<int, SolarTimer>(TimerDefaultSize);

        m_TimersToBeAdded = new List<SolarTimer>(TimerDefaultSize);
        m_TimersToBeRemoved = new List<SolarTimer>(TimerDefaultSize);
    }

    int CalcTimerID()
    {
        if (IncreaseTimerID > int.MaxValue)
        {
            IncreaseTimerID = DefaultTimerID;
        }

        return IncreaseTimerID++;
    }

    int CreateTimer(int times, float delayTime, float interval, UnityAction callback)
    {
        SolarTimer timer = m_TimerPool.Pop();
        
        int newTimerID = CalcTimerID();

        if (timer == null)
        {
            timer = new SolarTimer(newTimerID, times, delayTime, interval, callback);
        }
        else
        {
            timer.Set(newTimerID, times, delayTime, interval, callback);
        }

        m_TimersToBeAdded.Add(timer);

        return newTimerID;
    }

    /// <summary>
    /// 创建只会执行1次的Timer
    /// </summary>
    /// <param name="delayTime">单位为秒</param>
    /// <param name="callback">回调</param>
    /// <returns></returns>
    public int CreateSingleTimer(float delayTime, UnityAction callback)
    {
        return CreateTimer(1, delayTime, 0, callback);
    }

    /// <summary>
    /// 创建执行多次的Timer
    /// </summary>
    /// <param name="times"></param>
    /// <param name="delayTime">秒</param>
    /// <param name="interval">秒</param>
    /// <param name="callback"></param>
    /// <returns></returns>
    public int CreateMultiTimer(int times, float delayTime, float interval, UnityAction callback)
    {
        return CreateTimer(times, delayTime, interval, callback);
    }

    /// <summary>
    /// 创建执行无限次的Timer
    /// </summary>
    /// <param name="delayTime">秒</param>
    /// <param name="interval">秒</param>
    /// <param name="callback"></param>
    /// <returns></returns>
    public int CreateForeverTimer(float delayTime, float interval, UnityAction callback)
    {
        return CreateTimer(SolarTimer.ForeverTimes, delayTime, interval, callback);
    }

    /// <summary>
    /// 更新所有Timer的状态
    /// </summary>
    /// <param name="dt"></param>
    public void Update(float dt)
    {
        // 先更新所有的Timer
        foreach (var item in m_Timers)
        {
            SolarTimer timer = item.Value;

            if (timer != null && timer.Update(dt) == SolarTimer.State.Over)
            {
                m_TimersToBeRemoved.Add(timer);
            }
        }

        // 再增加或删除Timer
        foreach (var addTimer in m_TimersToBeAdded)
        {
            m_Timers.Add(addTimer.ID, addTimer);
        }
        m_TimersToBeAdded.Clear();

        foreach (var removeTimer in m_TimersToBeRemoved)
        {
            m_Timers.Remove(removeTimer.ID);
        }
        m_TimersToBeRemoved.Clear();
    }

    private SolarTimer PeekTimer(int timerID)
    {
        if (timerID == InvalidTimerID)
        {
            return null;
        }

        SolarTimer timer = null;
        if (m_Timers.TryGetValue(timerID, out timer))
        {
            return timer;
        }

        return null;
    }

    /// <summary>
    /// Timer ID是否有效
    /// </summary>
    /// <param name="timerID"></param>
    /// <returns></returns>
    public bool IsTimerIDVallid(int timerID)
    {
        return PeekTimer(timerID) != null;
    }

    /// <summary>
    /// 获取指定Timer的时间进度
    /// </summary>
    /// <param name="timerID"></param>
    /// <returns></returns>
    public float GetTimerProgress(ref int timerID)
    {
        SolarTimer timer = PeekTimer(timerID);

        if (timer != null)
        {
            return timer.TimeProgress;
        }

        timerID = InvalidTimerID;
        return 0;
    }

    /// <summary>
    /// 获取指定Timer的剩余时间
    /// </summary>
    /// <param name="timerID"></param>
    /// <returns></returns>
    public float GetTimerLeft(ref int timerID)
    {
        SolarTimer timer = PeekTimer(timerID);

        if (timer != null)
        {
            return timer.TimeLeft;
        }

        timerID = InvalidTimerID;
        return 0;
    }

    /// <summary>
    /// 暂停指定计时器
    /// </summary>
    /// <param name="timerID"></param>
    public void PauseTimer(ref int timerID)
    {
        SolarTimer timer = PeekTimer(timerID);

        if (timer != null)
        {
            timer.Pause();
        }
        else
        {
            timerID = InvalidTimerID;
        }
    }

    /// <summary>
    /// 恢复计时器
    /// </summary>
    /// <param name="timerID"></param>
    public void ResumeTimer(ref int timerID)
    {
        SolarTimer timer = PeekTimer(timerID);

        if (timer != null)
        {
            timer.Resume();
        }
        else
        {
            timerID = InvalidTimerID;
        }
    }

    /// <summary>
    /// 停止指定计时器
    /// </summary>
    /// <param name="timerID"></param>
    public void StopTimer(ref int timerID)
    {
        SolarTimer timer = PeekTimer(timerID);

        if (timer != null)
        {
            timer.Over();
            m_TimersToBeRemoved.Add(timer);
        }
        else
        {
            timerID = InvalidTimerID;
        }
    }
}