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

    // Timer缓存池
    TimerPool m_TimerPool;

    // 当前所有活动的Timer
    List<SolarTimer> m_Timers;

    private SolarTimerManager()
    {
        m_TimerPool = new TimerPool();

        m_Timers = new List<SolarTimer>(10);
    }

    SolarTimer CreateTimer(int times, float delayTime, float interval, UnityAction callback)
    {
        SolarTimer timer = m_TimerPool.Pop(SolarTimer.TimerIdentifier);

        bool cache = (timer != null);

        if (timer == null)
        {
            timer = new SolarTimer(times, delayTime, interval, callback);
        }
        else
        {
            timer.Set(times, delayTime, interval, callback);
        }

        m_Timers.Add(timer);

        //SolarLogger.LogInfoFormat(eOutPutModule.General, "SolarTimerManager CreateTimer Cache:{0} Time:{1} Timer:{2}", cache, Time.time, timer);

        return timer;
    }

    /// <summary>
    /// 创建只会执行1次的Timer
    /// </summary>
    /// <param name="delayTime"></param>
    /// <param name="callback"></param>
    /// <returns></returns>
    public SolarTimer CreateSingleTimer(float delayTime, UnityAction callback)
    {
        SolarTimer timer = CreateTimer(1, delayTime, 0, callback);

        return timer;
    }

    /// <summary>
    /// 创建执行多次的Timer
    /// </summary>
    /// <param name="times"></param>
    /// <param name="delayTime"></param>
    /// <param name="interval"></param>
    /// <param name="callback"></param>
    /// <returns></returns>
    public SolarTimer CreateMultiTimer(int times, float delayTime, float interval, UnityAction callback)
    {
        SolarTimer timer = CreateTimer(times, delayTime, interval, callback);

        return timer;
    }

    /// <summary>
    /// 创建执行无限次的Timer
    /// </summary>
    /// <param name="delayTime"></param>
    /// <param name="interval"></param>
    /// <param name="callback"></param>
    /// <returns></returns>
    public SolarTimer CreateForeverTimer(float delayTime, float interval, UnityAction callback)
    {
        SolarTimer timer = CreateTimer(SolarTimer.ForeverTimes, delayTime, interval, callback);

        return timer;
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
    /// 删除对应Timer
    /// </summary>
    /// <param name="timerToBeRemoved"></param>
    public void RemoveTimer(SolarTimer timerToBeRemoved)
    {
        if (timerToBeRemoved == null)
        {
            return;
        }

        m_TimerPool.Push(timerToBeRemoved);

        m_Timers.Remove(timerToBeRemoved);
    }
}