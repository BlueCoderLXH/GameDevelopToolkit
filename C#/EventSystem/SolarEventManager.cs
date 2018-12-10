using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

/// <summary>
/// 事件管理
/// 
/// 主要用于全局事件的注册/反注册/分发
/// </summary>
public class SolarEventManager
{
    /// <summary>
    /// 这里采用UnityEvent而不是C#原生的delegate, 原因是Unity2017之后对UnityEvent做了优化, 
    /// 只会在首次AddListener时有少量GC, 其余操作都不会有
    /// </summary>
    private class SolarEventListener : UnityEvent<SolarEvent> { }

    static SolarEventManager _instance = new SolarEventManager();
    public static SolarEventManager Instance
    {
        get { return _instance; }
    }

    private Dictionary<SolarEventID, SolarEventListener> m_EventListeners;

    private SolarEventManager()
    {
        int eventsCacheCount = SolarEventID.Event_ID_End - SolarEventID.Event_ID_Start - 1;
        m_EventListeners = new Dictionary<SolarEventID, SolarEventListener>(eventsCacheCount);
    }

    public void Register(SolarEventID id, UnityAction<SolarEvent> callback)
    {
        SolarEventListener eventListener = null;
        if (!m_EventListeners.TryGetValue(id, out eventListener))
        {
            eventListener = new SolarEventListener();
            m_EventListeners.Add(id, eventListener);
        }

        if (eventListener == null)
        {
            Debug.LogFormat("[SolarEventManager] Try to register '{0}' event failed!", id);
            return;
        }

        eventListener.AddListener(callback);
    }

    public void Unregister(SolarEventID id, UnityAction<SolarEvent> callback)
    {
        SolarEventListener eventListeners;
        if (m_EventListeners.TryGetValue(id, out eventListeners))
        {
            eventListeners.RemoveListener(callback);
        }

        if (eventListeners == null)
        {
            Debug.LogFormat("[SolarEventManager] Try to unregister '{0}' event failed!", id);
        }
    }

    public void Dispatch(SolarEvent e)
    {
        SolarEventListener eventListeners;
        if (m_EventListeners.TryGetValue(e.ID, out eventListeners))
        {
            eventListeners.Invoke(e);
        }

        if (eventListeners == null)
        {
            Debug.LogFormat("[SolarEventManager] Try to dispatch '{0}' event failed!", e.ID);
        }
    }

    public void Release()
    {
        if (m_EventListeners != null)
        {
            m_EventListeners.Clear();
        }

        _instance = null;
    }
}