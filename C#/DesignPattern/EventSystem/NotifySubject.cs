using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

/// <summary>
/// 事件通知 (采用Observer模式)
/// </summary>
public class NotifySubject
{
    protected class NotifyObserver : UnityEvent<IEventData> { }

    protected Dictionary<EventID, NotifyObserver> m_Observers;

    public NotifySubject(int cacheEventIDStart, int cacheEventIDEnd)
    {
        int unitEventCount = cacheEventIDEnd - cacheEventIDStart;

        m_Observers = new Dictionary<EventID, NotifyObserver>(unitEventCount);

        for (int i = cacheEventIDStart; i < cacheEventIDEnd; i++)
        {
            m_Observers.Add((EventID)i, new NotifyObserver());
        }
    }

    public void AddObserver(EventID eventID, UnityAction<IEventData> callback)
    {
        NotifyObserver observer = null;
        if (m_Observers.TryGetValue(eventID, out observer))
        {
            observer.AddListener(callback);
        }
    }

    public void RemoveObserver(EventID eventID, UnityAction<IEventData> callback)
    {
        NotifyObserver observer = null;
        if (m_Observers.TryGetValue(eventID, out observer))
        {
            observer.RemoveListener(callback);
        }
    }

    public void Notify(EventID eventID, IEventData eventData)
    {
        NotifyObserver observer = null;
        if (m_Observers.TryGetValue(eventID, out observer))
        {
            observer.Invoke(eventData);
        }
    }

    public void ClearObservers()
    {
        foreach (var observerItem in m_Observers)
        {
            observerItem.Value.RemoveAllListeners();
        }

        m_Observers.Clear();
    }
}