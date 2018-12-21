using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

/// <summary>
/// 计时器(外部不要自己new, 通过SolarTimerManager提供的接口创建)
/// </summary>
public class SolarTimer : Poolable
{
    public const int ForeverTimes = -1;
    public const int TimerIdentifier = 0;

    public enum State
    {
        Running,
        Pause,
        Over
    }


    // 回调次数(-1表示无限次)
    private int m_Times;

    // 初次回调延时时间(s)
    private float m_DelayTime;

    // 回调间隔时间
    private float m_IntervalTime;

    private ulong m_TimerID;              // timer id
    public ulong ID
    {
        get { return m_TimerID; }
    }

    private State m_State;               // 计时器状态

    private float m_CurTimeCount;        // 当前计时

    private int m_CurRunTimes;           // 当前回调次数

    private UnityAction m_TimerCallback; // 回调函数

    public SolarTimer(ulong timerID, int times, float delayTime, float interval, UnityAction callback) : base(TimerIdentifier)
    {
        Set(timerID, times, delayTime, interval, callback);
    }

    public void Set(ulong timerID, int times, float delayTime, float interval, UnityAction callback)
    {
        m_TimerID = timerID;

        m_Times = times;
        m_DelayTime = delayTime;
        m_IntervalTime = interval;
        m_TimerCallback = callback;

        m_State = State.Running;

        m_CurTimeCount = 0;
        m_CurRunTimes = 0;
    }

    /// <summary>
    /// 暂停计时器
    /// </summary>
    public void Pause()
    {
        if (m_State == State.Over)
        {
            return;
        }

        m_State = State.Pause;
    }

    /// <summary>
    /// 结束计时器
    /// </summary>
    public void Over()
    {
        if (m_State == State.Over)
        {
            return;
        }

        m_State = State.Over;
    }

    /// <summary>
    /// 更新计时器状态(只能在SolarTimerManager中被调用)
    /// </summary>
    /// <param name="dt"></param>
    /// <returns></returns>
    public State Update(float dt)
    {
        if (m_State == State.Pause ||
            m_State == State.Over)
        {
            return m_State;
        }

        m_CurTimeCount += dt;

        // 计时检查时间(初次为delay, 之后为interval)
        float checkTime = m_IntervalTime;
        if (m_CurRunTimes == 0)
        {
            checkTime = m_DelayTime;
        }

        // 计时到时, 回调
        if (m_CurTimeCount >= checkTime)
        {
            m_CurTimeCount -= checkTime;

            m_CurRunTimes += (m_Times != ForeverTimes) ? 1 : 0;

            m_TimerCallback.Invoke();

            //SolarLogger.LogInfoFormat(eOutPutModule.General, "SolarTimer Invoke Time:{0} Timer:{1}", Time.time, this);
        }

        // 回调有限次数, 结束计时器
        if (m_Times != ForeverTimes && m_CurRunTimes >= m_Times)
        {
            m_State = State.Over;
        }

        return m_State;
    }

    public override string ToString()
    {
        return string.Format("[ID:{0} Times:{1} Delay:{2} Interval:{3}]", (m_TimerID << 16) >> 16, m_Times, m_DelayTime, m_IntervalTime);
    }
}