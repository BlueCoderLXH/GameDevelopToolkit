/// <summary>
/// 自定义事件数据(抽象接口)
/// </summary>
public interface IEventData
{
}

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

/// <summary>
/// 事件数据工厂
/// (创建事件数据对象统一使用工厂方法, 代码更加简洁)
/// </summary>
public sealed class EventDataFactory
{
    private EventDataFactory() { }

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