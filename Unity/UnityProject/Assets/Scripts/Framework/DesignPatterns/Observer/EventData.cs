namespace Framework.DesignPattern
{
    #region Generic-Typed Event Data
    /*
     * 事件数据采用模板
     * 
     * 优点:
     *     不需要数据格式模板可以重用
     * 缺点:
     *     1、事件数据使用处需要关心具体的事件类型, 进行强转, 不安全
     *     2、模板的成员都是统一的名字'Param_x', 可读性不高
     */

    public struct EventData0 : IEventData
    {
    }

    public struct EventData1<T> : IEventData
    {
        public T Param1;

        public EventData1(T param1)
        {
            Param1 = param1;
        }
    }

    public struct EventData2<T1, T2> : IEventData
    {
        public T1 Param1;
        public T2 Param2;

        public EventData2(T1 param1, T2 param2)
        {
            Param1 = param1;
            Param2 = param2;
        }
    }

    public struct EventData3<T1, T2, T3> : IEventData
    {
        public T1 Param1;
        public T2 Param2;
        public T3 Param3;

        public EventData3(T1 param1, T2 param2, T3 param3)
        {
            Param1 = param1;
            Param2 = param2;
            Param3 = param3;
        }
    }

    public struct EventData4<T1, T2, T3, T4> : IEventData
    {
        public T1 Param1;
        public T2 Param2;
        public T3 Param3;
        public T4 Param4;

        public EventData4(T1 param1, T2 param2, T3 param3, T4 param4)
        {
            Param1 = param1;
            Param2 = param2;
            Param3 = param3;
            Param4 = param4;
        }
    }

    public struct EventData5<T1, T2, T3, T4, T5> : IEventData
    {
        public T1 Param1;
        public T2 Param2;
        public T3 Param3;
        public T4 Param4;
        public T5 Param5;

        public EventData5(T1 param1, T2 param2, T3 param3, T4 param4, T5 param5)
        {
            Param1 = param1;
            Param2 = param2;
            Param3 = param3;
            Param4 = param4;
            Param5 = param5;
        }
    }
    #endregion



    #region Specific-Typed Event Data
    /*
     * 采用具体类型的事件数据格式
     * 
     * 优点: 
     *     1、事件数据使用处不需要关心具体的事件类型, 不需要进行强转, 类型安全
     *     2、自定义事件数据的格式以及成员名字, 可读性高
     * 缺点:
     *     每一个事件都需要手动单独定义一个对应的事件数据类型
     */

    public struct TestEvent1Data : IEventData
    {
        public string Name;
        public int Age;

        public override string ToString()
        {
            return string.Format($"TestEventData Name{Name}, Age:{Age}");
        }
    }

    public struct TestEvent2Data : IEventData
    {
        public string Name;
        public int Age;

        public override string ToString()
        {
            return string.Format($"TestEventData Name{Name}, Age:{Age}");
        }
    }
    #endregion
}