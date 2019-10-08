
namespace Framework.DesignPattern
{
    using System;

    /// <summary>
    /// 事件类型指定属性
    /// 1、用于注释以及事件数据类型检查的功能
    /// 2、建议事件数据类型检查的功能只在Debug模式下使用, Release中应该去掉
    /// </summary>
    internal class EventTypeAttribute : Attribute
    {
        Type m_DataType;
        public Type DataType => m_DataType;

        public EventTypeAttribute(Type dataType)
        {
            m_DataType = dataType;
        }
    }

    /// <summary>
    /// 事件枚举
    /// 注: 每新加一个新事件, 要按照规范写相关注释, 方便其他人能很快了解并正确使用该事件
    /// </summary>
    public enum GEventID
    {
        Begin_ID = -1,

        [EventType(typeof(TestEvent1Data))]
        TestEvent1,

        End_ID
    }
}