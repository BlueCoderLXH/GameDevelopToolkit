namespace Framework.DesignPattern
{
    using System.Collections.Generic;

    public delegate void ObserverCallback(IEventData eData);

    /// <summary>
    /// 观察者事件集合
    /// 1、C#自带的代理回调进行+=或有GC
    /// 2、采用LinkedList链表存储事件回调, 增删速度快, 减少不必要的GC开销
    /// </summary>
    public class ObserverEventList
    {
        LinkedList<ObserverCallback> m_Events;

        public ObserverEventList()
        {
            m_Events = new LinkedList<ObserverCallback>();
        }

        public void AddListener(ObserverCallback callback)
        {
            if (m_Events != null)
            {
                m_Events.AddLast(callback);
            }
        }

        public void RemoveListener(ObserverCallback callback)
        {
            if (m_Events != null)
            {
                m_Events.Remove(callback);
            }
        }

        public void Invoke(IEventData eventData)
        {
            foreach (var eventItem in m_Events)
            {
                eventItem.Invoke(eventData);
            }
        }

        public void RemoveAllListeners()
        {
            if (m_Events != null)
            {
                m_Events.Clear();
                m_Events = null;
            }
        }
    }

    /// <summary>
    /// 事件通知 (采用Observer模式)
    /// </summary>
    public class NotifySubject
    {
        protected Dictionary<int, ObserverEventList> m_Observers;

        public NotifySubject() { }

        public void AddObserver(int eventID, ObserverCallback observer)
        {
            ObserverEventList observerEvents = null;
            if (!m_Observers.TryGetValue(eventID, out observerEvents))
            {
                observerEvents = new ObserverEventList();
                m_Observers.Add(eventID, observerEvents);
            }

            observerEvents.AddListener(observer);
        }

        public void RemoveObserver(int eventID, ObserverCallback observer)
        {
            ObserverEventList observerEvents = null;
            if (m_Observers.TryGetValue(eventID, out observerEvents))
            {
                observerEvents.RemoveListener(observer);
            }
        }

        public void Notify(int eventID, IEventData eventData)
        {
            ObserverEventList observerEvents = null;
            if (m_Observers.TryGetValue(eventID, out observerEvents))
            {
                observerEvents.Invoke(eventData);
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
}