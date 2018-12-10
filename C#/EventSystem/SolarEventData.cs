/// <summary>
/// 自定义事件数据(抽象接口)
/// </summary>
public interface ISolarEventData
{
}

public struct SolarEventData_Param0 : ISolarEventData
{
}

public struct SolarEventData_Param1<T> : ISolarEventData
{
    public T Param;

    public SolarEventData_Param1(T param1)
    {
        Param = param1;
    }
}

public struct SolarEventData_Param2<T1, T2> : ISolarEventData
{
    public T1 Param1;
    public T2 Param2;

    public SolarEventData_Param2(T1 param1, T2 param2)
    {
        Param1 = param1;
        Param2 = param2;
    }
}

public struct SolarEventData_Param3<T1, T2, T3> : ISolarEventData
{
    public T1 Param1;
    public T2 Param2;
    public T3 Param3;

    public SolarEventData_Param3(T1 param1, T2 param2, T3 param3)
    {
        Param1 = param1;
        Param2 = param2;
        Param3 = param3;
    }
}

public struct SolarEventData_Param4<T1, T2, T3, T4> : ISolarEventData
{
    public T1 Param1;
    public T2 Param2;
    public T3 Param3;
    public T4 Param4;

    public SolarEventData_Param4(T1 param1, T2 param2, T3 param3, T4 param4)
    {
        Param1 = param1;
        Param2 = param2;
        Param3 = param3;
        Param4 = param4;
    }
}

public struct SolarEventData_Param5<T1, T2, T3, T4, T5> : ISolarEventData
{
    public T1 Param1;
    public T2 Param2;
    public T3 Param3;
    public T4 Param4;
    public T5 Param5;

    public SolarEventData_Param5(T1 param1, T2 param2, T3 param3, T4 param4, T5 param5)
    {
        Param1 = param1;
        Param2 = param2;
        Param3 = param3;
        Param4 = param4;
        Param5 = param5;
    }
}

public struct SolarEventData_Param6<T1, T2, T3, T4, T5, T6> : ISolarEventData
{
    public T1 Param1;
    public T2 Param2;
    public T3 Param3;
    public T4 Param4;
    public T5 Param5;
    public T6 Param6;

    public SolarEventData_Param6(T1 param1, T2 param2, T3 param3, T4 param4, T5 param5, T6 param6)
    {
        Param1 = param1;
        Param2 = param2;
        Param3 = param3;
        Param4 = param4;
        Param5 = param5;
        Param6 = param6;
    }
}