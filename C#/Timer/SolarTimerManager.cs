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

    // TimerID的最大值
    const ulong TimerIDMaxValue = 281474976710655;

    // 默认Timer ID的起点
    const ulong DefaultTimerID = 10000;

    // 无效的TimerID
    const ulong InvalidTimerID = 0;

    // 无效的Timer下标
    const int InvalidTimerIndex = -1;

    // 递增的Timer ID
    static ulong IncreaseTimerID = DefaultTimerID;

    // Timer缓存池
    TimerPool m_TimerPool;

    // 当前所有活动的Timer
    List<SolarTimer> m_Timers;

    // 存储已经删除的Timer下标
    List<int> m_RemovedTimerIndexs;

    private SolarTimerManager()
    {
        m_TimerPool = new TimerPool();

        m_Timers = new List<SolarTimer>(10);

        m_RemovedTimerIndexs = new List<int>(10);
    }

    /// <summary>
    /// 创建Timer ID(有两部分构成: Timer在m_Timer中的下标index(左16位)和IncreaseTimerID(右48位))
    /// </summary>
    /// <returns></returns>
    ulong CalcTimerID(ref int newTimerIndex)
    {
        if (IncreaseTimerID > TimerIDMaxValue)
        {
            IncreaseTimerID = DefaultTimerID;
        }

        ulong newTimerID = IncreaseTimerID++;

        if (m_Timers.Count > ushort.MaxValue)
        {
            SolarLogger.LogInfo(eOutPutModule.General, "SolarTimerManager: Create too much timers(over 65535)!");

            return DefaultTimerID;
        }

        ulong nextTimerIndex;

        // 当前m_Timers没有空位, 需要在m_Timers后面添加
        if (m_RemovedTimerIndexs.Count == 0)
        {
            nextTimerIndex = (ulong)m_Timers.Count;
        }
        // 有空位优先利用空位
        else
        {
            nextTimerIndex = (ulong)m_RemovedTimerIndexs[m_RemovedTimerIndexs.Count - 1];

            m_RemovedTimerIndexs.RemoveAt(m_RemovedTimerIndexs.Count - 1);

            newTimerIndex = (int)nextTimerIndex;
        }

        ulong realTimerID = (nextTimerIndex << TimerIDMaxBit) | newTimerID;

        return realTimerID;
    }

    ulong CreateTimer(int times, float delayTime, float interval, UnityAction callback)
    {
        SolarTimer timer = m_TimerPool.Pop(SolarTimer.TimerIdentifier);

        //bool cache = (timer != null);

        int newTimerIndex = -1;

        ulong realTimerID = CalcTimerID(ref newTimerIndex);

        if (timer == null)
        {
            timer = new SolarTimer(realTimerID, times, delayTime, interval, callback);
        }
        else
        {
            timer.Set(realTimerID, times, delayTime, interval, callback);
        }

        if (newTimerIndex < 0 || newTimerIndex >= m_Timers.Count)
        {
            m_Timers.Add(timer);
        }
        else
        {
            m_Timers[newTimerIndex] = timer;
        }

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

            if (timer != null && timer.Update(dt) == SolarTimer.State.Over)
            {
                RemoveTimerAt(i);
            }
        }
    }

    /// <summary>
    /// 删除对应下标的Timer
    /// </summary>
    /// <param name="removeIndex"></param>
    void RemoveTimerAt(int removeIndex)
    {
        // 'm_Timers[removeIndex] == null' 用于避免在Timer的Callback中StopTimer, 以导致重复性删除
        if (removeIndex < 0 || removeIndex >= m_Timers.Count || m_Timers[removeIndex] == null)
        {
            return;
        }

        SolarTimer timerToBeRemoved = m_Timers[removeIndex];

        m_TimerPool.Push(timerToBeRemoved);

        /*
         *     这里直接置空就好, 空位可以重复利用(算法效率更高); 不能调用m_Timers.RemoveAt, 因为
         * 这样会破坏m_Timers的存储结构, 导致无法正常获取removeIndex后面的Timer
         */
        m_Timers[removeIndex] = null;

        // 标记空位
        m_RemovedTimerIndexs.Add(removeIndex);
    }

    /// <summary>
    /// 根据Timer ID获取Timer的下标
    /// </summary>
    /// <param name="timerID"></param>
    /// <returns></returns>
    int PeekTimer(ulong timerID)
    {
        int timerIndex = (int)(timerID >> TimerIDMaxBit);

        SolarTimer peekTimer = null;
        if (timerIndex >= 0 && timerIndex < m_Timers.Count)
        {
            peekTimer = m_Timers[timerIndex];

            // invalid timer id
            if (peekTimer == null || peekTimer.ID != timerID)
            {
                timerIndex = InvalidTimerIndex;
            }
        }

        return timerIndex;
    }

    /// <summary>
    /// 判断timerID是否合法
    /// </summary>
    /// <param name="timerID"></param>
    /// <returns></returns>
    public bool IsTimerIDVallid(ulong timerID)
    {
        return timerID != InvalidTimerID && PeekTimer(timerID) != InvalidTimerIndex;
    }

    /// <summary>
    /// 暂停指定计时器
    /// </summary>
    /// <param name="timerID"></param>
    public void PauseTimer(ref ulong timerID)
    {
        int timerIndexToBePaused = PeekTimer(timerID);

        if (timerIndexToBePaused != InvalidTimerIndex)
        {
            m_Timers[timerIndexToBePaused].Pause();

            //SolarLogger.LogInfoFormat(eOutPutModule.General, "SolarTimerManager StopTimer Timer:{0}", m_Timers[timerIndexToBePaused]);
        }
        else
        {
            timerID = InvalidTimerID;
        }
    }

    /// <summary>
    /// 停止指定计时器(计时器会在下一帧被删除)
    /// </summary>
    /// <param name="timerID"></param>
    public void StopTimer(ref ulong timerID)
    {
        int timerIndexToBeStoped = PeekTimer(timerID);

        if (timerIndexToBeStoped != InvalidTimerIndex)
        {
            m_Timers[timerIndexToBeStoped].Over();

            //SolarLogger.LogInfoFormat(eOutPutModule.General, "SolarTimerManager StopTimer Timer:{0}", m_Timers[timerIndexToBeStoped]);

            RemoveTimerAt(timerIndexToBeStoped);
        }

        timerID = InvalidTimerID;
    }
}