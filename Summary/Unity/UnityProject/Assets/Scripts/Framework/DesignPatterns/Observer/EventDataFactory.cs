namespace Framework.DesignPattern
{
    /// <summary>
    /// 事件数据工厂
    /// (创建事件数据对象统一使用工厂方法, 代码更加简洁)
    /// </summary>
    public sealed class EventDataFactory
    {
        private EventDataFactory() { }

        // 可以用null代替
        public static IEventData Create()
        {
            return new EventData0();
        }

        public static IEventData Create<T1>(T1 param1)
        {
            return new EventData1<T1>(param1);
        }

        public static IEventData Create<T1, T2>(T1 param1, T2 param2)
        {
            return new EventData2<T1, T2>(param1, param2);
        }

        public static IEventData Create<T1, T2, T3>(T1 param1, T2 param2, T3 param3)
        {
            return new EventData3<T1, T2, T3>(param1, param2, param3);
        }

        public static IEventData Create<T1, T2, T3, T4>(T1 param1, T2 param2, T3 param3, T4 param4)
        {
            return new EventData4<T1, T2, T3, T4>(param1, param2, param3, param4);
        }

        public static IEventData Create<T1, T2, T3, T4, T5>(T1 param1, T2 param2, T3 param3, T4 param4, T5 param5)
        {
            return new EventData5<T1, T2, T3, T4, T5>(param1, param2, param3, param4, param5);
        }
    }
}
