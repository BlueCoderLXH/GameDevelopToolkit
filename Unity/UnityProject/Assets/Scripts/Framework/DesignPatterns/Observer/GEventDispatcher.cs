namespace Framework.DesignPattern
{
    using System;
    using System.Reflection;

    /// <summary>
    /// 全局事件通知中心
    /// 
    /// 提供两种类型的事件数据:
    ///     1、模板化类型
    ///     2、具体类型
    /// 
    /// Tips:
    ///     在开发中过程中, 采用具体类型的事件数据类型会更优, 良好的可读性以及类型安全性显得更加重要
    /// </summary>
    public static class GEventDispatcher
    {
        static EventDispatcher<GEventID> m_EventDispatcher;

        static GEventDispatcher()
        {
            m_EventDispatcher = new EventDispatcher<GEventID>((int)GEventID.End_ID);
        }

        #region Generic-Typed Event Data
        /*
         * 接受模板化类型的事件数据
         * 
         * 优点:
         *     事件数据采用模板类型, 模板代码可以复用
         * 缺点:
         *     1、事件监听处需要关心具体的事件数据类型, 进行强转, 不安全
         *     2、模板的成员都是统一的名字'Param_x', 可读性不高
         */
        //public static void Register(GEventID eventID, EventListener<IEventData> listener)
        //{
        //    m_EventDispatcher?.RegisterListener(eventID, listener);
        //}

        //public static void Unregister(GEventID eventID, EventListener<IEventData> listener)
        //{
        //    m_EventDispatcher?.UnregisterListener(eventID, listener);
        //}

        //public static void Dispatch(GEventID eventID, IEventData eventData)
        //{
        //    m_EventDispatcher?.Dispatch(eventID, eventData);
        //}
        #endregion

        #region Specific -Typed Event Data
        /*
         * 接受具体类型的事件数据
         * 
         * 优点: 
         *     1、事件监听处不需要关心具体的事件数据类型, 不需要进行强转, 类型安全
         *     2、自定义事件数据的格式以及成员名字, 可读性高
         * 缺点:
         *     每一个事件都需要手动单独定义一个对应的事件数据类型
         *     (如果有效控制事件的数量, 这个方式还是能接受的)
         */
        public static void Register<TEventDataType>(GEventID eventID, EventListener<TEventDataType> listener)
            where TEventDataType : IEventData
        {
            m_EventDispatcher?.RegisterListener(eventID, listener);
        }

        public static void Unregister<TEventDataType>(GEventID eventID, EventListener<TEventDataType> listener)
            where TEventDataType : IEventData
        {
            m_EventDispatcher?.UnregisterListener(eventID, listener);
        }

        public static void Dispatch<TEventDataType>(GEventID eventID, TEventDataType eventData)
            where TEventDataType : IEventData
        {
            m_EventDispatcher?.Dispatch(eventID, eventData);
        }
        #endregion

        public static void Release()
        {
            m_EventDispatcher?.RemoveAll();
            m_EventDispatcher = null;
        }
    }
}