using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

/// <summary>
/// 计时器管理
/// 
/// 计时器的创建/缓存/更新/删除
/// </summary>
public class SolarTimerManager
{
    private class TimerPool : ManagedPool<SolarTimer> { }

    static SolarTimerManager m_Instance;
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

    // 最多支持2^(48)个timer
    const int TimerIDMaxBit = 48;

    // 递增的Timer ID
    static ulong IncreaseTimerID = 0;

    // Timer缓存池
    TimerPool m_TimerPool;

    // 当前所有活动的Timer
    List<SolarTimer> m_Timers;

    private SolarTimerManager()
    {
        m_TimerPool = new TimerPool();

        m_Timers = new List<SolarTimer>(10);
    }

    /// <summary>
    /// 创建Timer ID(有两部分构成: Timer在m_Timer中的下标index(左16位)和IncreaseTimerID(右48位))
    /// </summary>
    /// <returns></returns>
    ulong CalcTimerID()
    {
        ulong newTimerID = IncreaseTimerID++;

        if (m_Timers.Count > ushort.MaxValue)
        {
            throw new System.Exception("SolarTimerManager: Create too much timers(over 65535)!");
        }

        ulong realTimerID = (((ulong)m_Timers.Count) << TimerIDMaxBit) + newTimerID;

        return realTimerID;
    }

    ulong CreateTimer(int times, float delayTime, float interval, UnityAction callback)
    {
        SolarTimer timer = m_TimerPool.Pop(SolarTimer.TimerIdentifier);

        //bool cache = (timer != null);

        ulong realTimerID = CalcTimerID();

        if (timer == null)
        {
            timer = new SolarTimer(realTimerID, times, delayTime, interval, callback);
        }
        else
        {
            timer.Set(realTimerID, times, delayTime, interval, callback);
        }

        m_Timers.Add(timer);

        //SolarLogger.LogInfoFormat(eOutPutModule.General, "SolarTimerManager CreateTimer Cache:{0} Time:{1} Timer:{2}", cache, Time.time, timer);

        return realTimerID;
    }

    /// <summary>
    /// 创建只会执行1次的Timer
    /// </summary>
    /// <param name="delayTime"></param>
    /// <param name="callback"></param>
    /// <returns></returns>
    public ulong CreateSingleTimer(float delayTime, UnityAction callback)
    {
        return CreateTimer(1, delayTime, 0, callback);
    }

    /// <summary>
    /// 创建执行多次的Timer
    /// </summary>
    /// <param name="times"></param>
    /// <param name="delayTime"></param>
    /// <param name="interval"></param>
    /// <param name="callback"></param>
    /// <returns></returns>
    public ulong CreateMultiTimer(int times, float delayTime, float interval, UnityAction callback)
    {
        return CreateTimer(times, delayTime, interval, callback);
    }

    /// <summary>
    /// 创建执行无限次的Timer
    /// </summary>
    /// <param name="delayTime"></param>
    /// <param name="interval"></param>
    /// <param name="callback"></param>
    /// <returns></returns>
    public ulong CreateForeverTimer(float delayTime, float interval, UnityAction callback)
    {
        return CreateTimer(SolarTimer.ForeverTimes, delayTime, interval, callback);
    }

    /// <summary>
    /// 更新所有Timer的状态
    /// </summary>
    /// <param name="dt"></param>
    public void Update(float dt)
    {
        for (int i = m_Timers.Count - 1; i >= 0; i--)
        {
            SolarTimer timer = m_Timers[i];

            if (timer.Update(dt) == SolarTimer.State.Over)
            {
                RemoveTimer(i);
            }
        }
    }

    /// <summary>
    /// 删除对应下标的Timer
    /// </summary>
    /// <param name="removeIndex"></param>
    void RemoveTimer(int removeIndex)
    {
        if (removeIndex < 0 || removeIndex >= m_Timers.Count)
        {
            return;
        }

        SolarTimer timerToBeRemoved = m_Timers[removeIndex];

        m_TimerPool.Push(timerToBeRemoved);

        m_Timers.RemoveAt(removeIndex);
    }

    /// <summary>
    /// 根据Timer ID获取Timer对象
    /// </summary>
    /// <param name="timerID"></param>
    /// <returns></returns>
    SolarTimer PeekTimer(ulong timerID)
    {
        int timerIndex = (int)(timerID >> TimerIDMaxBit);

        SolarTimer peekTimer = null;
        if (timerIndex >= 0 && timerIndex < m_Timers.Count)
        {
            peekTimer = m_Timers[timerIndex];

            // invalid timer id
            if (peekTimer.ID != timerID)
            {
                peekTimer = null;
            }
        }

        return peekTimer;
    }

    ///// <summary>
    ///// 删除对应Timer
    ///// </summary>
    //public void RemoveTimer(ulong timerID)
    //{
    //    SolarTimer timerToBeRemoved = PeekTimer(timerID);

    //    if (timerToBeRemoved != null)
    //    {
    //        timerToBeRemoved.Over();

    //        m_Timers.Remove(timerToBeRemoved);
    //    }
    //}

    /// <summary>
    /// 暂停指定计时器
    /// </summary>
    /// <param name="timerID"></param>
    public void PauseTimer(ulong timerID)
    {
        SolarTimer timerToBePaused = PeekTimer(timerID);

        if (timerToBePaused != null)
        {
            timerToBePaused.Pause();
        }
    }

    /// <summary>
    /// 停止指定计时器(计时器会在下一帧被删除)
    /// </summary>
    /// <param name="timerID"></param>
    public void StopTimer(ulong timerID)
    {
        SolarTimer timerToBeStoped = PeekTimer(timerID);

        if (timerToBeStoped != null)
        {
            timerToBeStoped.Over();
        }
    }
}