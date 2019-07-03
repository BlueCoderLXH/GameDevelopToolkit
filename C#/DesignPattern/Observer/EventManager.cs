namespace Framework.DesignPattern
{
    /// <summary>
    /// 事件管理
    /// 主要用于全局事件的注册/反注册/分发
    /// </summary>
    public class EventManager : Singleton<EventManager>
    {
        private NotifySubject m_GlobalEventNotify;

        private EventManager()
        {
            m_GlobalEventNotify = new NotifySubject();
        }

        public void Register(EventID eventID, ObserverCallback callback)
        {
            m_GlobalEventNotify.AddObserver((int)eventID, callback);
        }

        public void Unregister(EventID eventID, ObserverCallback callback)
        {
            m_GlobalEventNotify.RemoveObserver((int)eventID, callback);
        }

        public void Dispatch(EventID eventID, IEventData eventData)
        {
            m_GlobalEventNotify.Notify((int)eventID, eventData);
        }

        public void Release()
        {
            m_GlobalEventNotify.ClearObservers();
            m_GlobalEventNotify = null;

            _instance = null;
        }
    }
}
