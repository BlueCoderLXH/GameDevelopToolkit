using System.Collections.Generic;
using UnityEngine.Events;

/// <summary>
/// 事件管理
/// 
/// 主要用于全局事件的注册/反注册/分发
/// </summary>
public class EventManager : Singleton<EventManager>
{
    private NotifySubject m_GlobalEventNotify;

    private EventManager()
    {
        int cacheEventIDStart = (int)EventID.Global_Event_ID_Start + 1;
        int cacheEventIDEnd = (int)EventID.Global_Event_ID_End;
        m_GlobalEventNotify = new NotifySubject(cacheEventIDStart, cacheEventIDEnd);
    }

    public void Register(EventID eventID, UnityAction<IEventData> callback)
    {
        m_GlobalEventNotify.AddObserver(eventID, callback);
    }

    public void Unregister(EventID eventID, UnityAction<IEventData> callback)
    {
        m_GlobalEventNotify.RemoveObserver(eventID, callback);
    }

    public void Dispatch(EventID eventID, IEventData eventData)
    {
        m_GlobalEventNotify.Notify(eventID, eventData);
    }

    public void Release()
    {
        m_GlobalEventNotify.ClearObservers();
        m_GlobalEventNotify = null;

        _instance = null;
    }
}