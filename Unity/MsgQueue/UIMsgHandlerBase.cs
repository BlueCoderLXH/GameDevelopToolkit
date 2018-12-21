using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

/// <summary>
/// UI消息类
/// </summary>
public class UIMessage
{
    static int Index = 0;

    public enum State
    {
        New,          // 新消息
        Pause,        // 消息暂停中
        Waiting,      // 消息等待中
        Showing,      // 消息展示中
        Finished,     // 已使用过的消息(即将被删除)
        WaitOverTime  // 等待超时(即将被删除)
    }

    int m_Index;

    // 展示内容
    string m_Content;

    // 展示停留时间(s)
    float m_ShowTime;

    // 最大等待时间(s), 等待超时消息无效, 不会再展示
    float m_MaxWaitTime;

    // 当前已等待时间(s)
    float m_CurWaitTime;

    // 当前已展示时间(s)
    float m_CurShowTime;

    // 消息的状态
    State m_State;

    public string Content
    {
        get { return m_Content; }
    }

    public State MsgState
    {
        get { return m_State; }
    }

    public UIMessage(string content, float showTime, float waitTime)
    {
        m_Index = Index++;

        m_Content = content;
        m_ShowTime = showTime;
        m_MaxWaitTime = waitTime;

        m_CurShowTime = 0;
        m_CurWaitTime = 0;

        m_State = State.New;
    }

    public void Show()
    {
        if (m_State == State.Finished)
        {
            return;
        }

        m_State = State.Showing;
    }

    public void Wait()
    {
        if (m_State == State.Finished)
        {
            return;
        }

        m_State = State.Waiting;
    }

    public void Pause()
    {
        if (m_State == State.Finished)
        {
            return;
        }

        m_State = State.Pause;
    }

    public void Update(float dt)
    {
        switch(m_State)
        {
            case State.Pause:
            case State.Finished:
                break;

            case State.New:
                Wait();
                break;

            case State.Waiting:
                OnWaiting(dt);
                break;

            case State.Showing:
                OnShowing(dt);
                break;
        }
    }

    void OnWaiting(float dt)
    {
        m_CurWaitTime += dt;

        if (m_CurWaitTime >= m_MaxWaitTime)
        {
            m_State = State.WaitOverTime;
            //SolarLogger.LogInfoFormat(eOutPutModule.UI, "UIMsgHandlerBase Message is WaitOverTime, Time(s):{0} {1}", Time.time, this);
        }
    }

    void OnShowing(float dt)
    {
        m_CurShowTime += dt;

        if (m_CurShowTime >= m_ShowTime)
        {
            m_State = State.Finished;
        }
    }

    public override string ToString()
    {
        return string.Format("Index:{0} ShowTime:{1} WaitTime:{2}", m_Index, m_ShowTime, m_MaxWaitTime);
    }
}

/// <summary>
/// UI通知消息处理基类
/// 
/// 注:不能实例化, 且子类不要重写Mono周期函数, 否则会覆盖基类的Awake/OnDestroy等,
/// 重载相应虚方法即可, 例如Init/Release
/// </summary>
public abstract class UIMsgHandlerBase : SolarUIBase
{
    [SerializeField] protected CanvasGroup m_CanvasGroup;
    [SerializeField] protected Text m_TextMsg;

    // 消息队列
    protected Queue<UIMessage> m_MsgQueue;

    // 当前处理的消息
    protected UIMessage m_CurUIMsg;

    void Awake()
    {
        Init();
    }

    void Update()
    {
        UpdateLogic();
    }

    void OnDestroy()
    {
        Release();
    }

    /// <summary>
    /// 初始化
    /// </summary>
    protected virtual void Init()
    {
        m_MsgQueue = new Queue<UIMessage>();

        if (m_CanvasGroup)
        {
            UIUtils.ShowCanvasGroup(m_CanvasGroup, false);
        }

        if (!m_TextMsg)
        {
            SolarLogger.LogErrorFormat(eOutPutModule.UI, "'{0}' message text component is missing!", name);
        }
    }

    /// <summary>
    /// 释放
    /// </summary>
    protected virtual void Release()
    {
    }

    /// <summary>
    /// 消息处理循环
    /// </summary>
    protected virtual void UpdateLogic()
    {
        // 更新所有消息的状态
        foreach (var msg in m_MsgQueue)
        {
            msg.Update(Time.deltaTime);
        }

        // 当前消息处理完毕
        if (m_CurUIMsg != null &&
            m_CurUIMsg.MsgState == UIMessage.State.Finished)
        {
            OnFinishMessage();

            m_CurUIMsg = null;
        }

        // 处理下一条消息(如果有)
        if (m_MsgQueue.Count > 0)
        {
            if (m_CurUIMsg == null ||
                m_CurUIMsg.MsgState == UIMessage.State.Finished)
            {
                HandleNextMessage();
            }
        }
    }

    /// <summary>
    /// 处理下一条消息
    /// </summary>
    void HandleNextMessage()
    {
        UIMessage nextMsg = null;

        while (m_MsgQueue.Count > 0)
        {
            nextMsg = m_MsgQueue.Peek();

            /*
             * 过滤已经处理过或超时的消息
             * 注: 之所以在这里过滤过期消息, 是因为队列的出口只有Dequeue一个
             */
            if (nextMsg.MsgState == UIMessage.State.Finished ||
                nextMsg.MsgState == UIMessage.State.WaitOverTime)
            {
                m_MsgQueue.Dequeue();
            }
            else
            {
                break;
            }

            nextMsg = null;
        }

        if (nextMsg != null)
        {
            m_CurUIMsg = nextMsg;

            OnHandleMessage();
        }
    }

    /// <summary>
    /// 添加一条消息
    /// </summary>
    /// <param name="eData"></param>
    protected void OnAddMsg(ISolarEventData eData)
    {
        EventData_ShowMessage eventData = (EventData_ShowMessage)eData;

        OnAddMsg(eventData);
    }

    /// <summary>
    /// 添加一条消息
    /// </summary>
    /// <param name="eData"></param>
    protected void OnAddMsg(EventData_ShowMessage eventData)
    {
        UIMessage newMsg = new UIMessage(eventData.Content, eventData.ShowStayTime, eventData.MaxWaitTime);

        m_MsgQueue.Enqueue(newMsg);

        //SolarLogger.LogInfoFormat(eOutPutModule.UI, "UIMsgHandlerBase OnAddMessage Time(s):{0} {1}", Time.time, newMsg);
    }

    /// <summary>
    /// 处理当前消息
    /// </summary>
    protected virtual void OnHandleMessage()
    {
        UIUtils.ShowCanvasGroup(m_CanvasGroup, true);

        if (m_TextMsg)
        {
            m_TextMsg.text = m_CurUIMsg.Content;
        }

        m_CurUIMsg.Show();

        //SolarLogger.LogInfoFormat(eOutPutModule.UI, "UIMsgHandlerBase OnHandleMessage Time(s):{0} {1}", Time.time, m_CurUIMsg);
    }

    /// <summary>
    /// 当前消息处理完毕
    /// </summary>
    protected virtual void OnFinishMessage()
    {
        UIUtils.ShowCanvasGroup(m_CanvasGroup, false);

        //SolarLogger.LogInfoFormat(eOutPutModule.UI, "UIMsgHandlerBase OnFinishMessage Time(s):{0} {1}", Time.time, m_CurUIMsg);
    }
}