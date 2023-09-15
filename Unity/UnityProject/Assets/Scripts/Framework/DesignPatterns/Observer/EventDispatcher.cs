namespace Framework.DesignPattern
{
    using System;
    using System.Reflection;
    using System.Collections.Generic;

    /// <summary>
    /// 事件数据(抽象接口)
    /// </summary>
    public interface IEventData { }

    /// <summary>
    /// 事件监听回调
    /// </summary>
    /// <param name="eventData"></param>
    public delegate void EventListener<T>(T eventData) where T : IEventData;

    /// <summary>
    /// 事件监听者集合接口
    /// </summary>
    public interface IEventListeners
    {
        void RemoveAll();
    }

    /// <summary>
    /// 事件监听者集合基类
    /// 管理所有监听某种特定事件的监听者
    /// 
    /// Tips:
    /// C#自带的delegate回调进行+=会有GC, 采用集合存储, 减少不必要的GC开销
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public abstract class EventListenersBase<TEventID, TListener> : IEventListeners
        where TEventID : System.Enum
    {
        protected List<TListener> m_Listeners;

        public EventListenersBase()
        {
            m_Listeners = new List<TListener>(4);
        }

        public void Add(TListener listener)
        {
            m_Listeners?.Add(listener);
        }

        public void Remove(TListener listener)
        {
            m_Listeners?.Remove(listener);
        }

        public void RemoveAll()
        {
            m_Listeners?.Clear();
            m_Listeners = null;
        }

        public abstract void Invoke(TEventID eventID, IEventData eventData);
    }

    /// <summary>
    /// 事件监听者集合
    /// </summary>
    /// <typeparam name="TEventID"></typeparam>
    /// <typeparam name="TEventData"></typeparam>
    public class EventListeners<TEventID, TEventData> : EventListenersBase<TEventID, EventListener<TEventData>>
        where TEventID : System.Enum
        where TEventData : IEventData
    {
        public override void Invoke(TEventID eventID, IEventData eventData)
        {
            foreach (var listener in m_Listeners)
            {
                if ((eventData is TEventData))
                {
                    listener((TEventData)eventData);
                }
                else
                {
                    throw new Exception($"EventType is invalid for event:{eventID}");
                }
            }
        }
    }

    /// <summary>
    /// 事件通知 (采用Observer模式)
    /// </summary>
    public class EventDispatcher<TEventID>
        where TEventID : System.Enum
    {
        protected Dictionary<TEventID, IEventListeners> m_EventListenerMap;

        public EventDispatcher(int cacheCount)
        {
            m_EventListenerMap = new Dictionary<TEventID, IEventListeners>(cacheCount);
        }

        public void RegisterListener<TEventDataType>(TEventID eventID, EventListener<TEventDataType> listener)
            where TEventDataType : IEventData
        {
            IEventListeners eventListeners = null;
            if (!m_EventListenerMap.TryGetValue(eventID, out eventListeners))
            {
                eventListeners = new EventListeners<TEventID, TEventDataType>();
                m_EventListenerMap.Add(eventID, eventListeners);
            }

            var listeners = eventListeners as EventListeners<TEventID, TEventDataType>;
            listeners.Add(listener);
        }

        public void UnregisterListener<TEventDataType>(TEventID eventID, EventListener<TEventDataType> listener)
            where TEventDataType : IEventData
        {
            IEventListeners eventListeners = null;
            if (m_EventListenerMap.TryGetValue(eventID, out eventListeners))
            {
                (eventListeners as EventListeners<TEventID, TEventDataType>).Remove(listener);
            }
        }

        public void Dispatch<TEventDataType>(TEventID eventID, TEventDataType eventData)
            where TEventDataType : IEventData
        {
#if EnableEventDataTypeValidation
            ValidateEventType(eventID, eventData);
#endif

            IEventListeners eventListeners = null;
            if (m_EventListenerMap.TryGetValue(eventID, out eventListeners))
            {
                (eventListeners as EventListeners<TEventID, TEventDataType>).Invoke(eventID, eventData);
            }
        }

        public void RemoveAll()
        {
            foreach (var observerItem in m_EventListenerMap)
            {
                observerItem.Value.RemoveAll();
            }

            m_EventListenerMap.Clear();
        }

#if EnableEventDataTypeValidation
        private static bool ValidateEventType<TEventDataType>(TEventID eventID, TEventDataType eventData)
        {
            var eventIDType = eventID.GetType();
            var eventIDInfo = eventIDType.GetField(eventID.ToString());

            var eventDataType = eventData.GetType();

            EventTypeAttribute eta = eventIDInfo.GetCustomAttribute<EventTypeAttribute>(true);
            if (eta == null)
                throw new Exception(string.Format($"Please add 'EventTypeAttribute' attribute for {eventIDType}.{eventID}"));

            bool validateFlag = eta.DataType.Equals(eventDataType);
            if (!validateFlag)
                throw new Exception(string.Format($"Wrong event type '{eventData.GetType()}' for {eventIDType}.{eventID}"));

            return validateFlag;
        }
#endif
    }
}