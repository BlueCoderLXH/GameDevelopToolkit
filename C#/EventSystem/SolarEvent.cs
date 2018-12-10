/// <summary>
/// 事件枚举
/// 
/// 注: 每新加一个新事件, 要按照规范写相关注释, 方便其他人能很快了解并正确使用该事件
/// </summary>
public enum SolarEventID
{
    Event_ID_Start,

    /// <summary>
    /// 游戏结束事件 参数是SolarEventData_Param1<int> int代表死法
    /// </summary>
    GameOver,

    Event_ID_End
}

/// <summary>
/// 自定义事件
/// </summary>
public struct SolarEvent
{
    /// <summary>
    /// 事件发起者
    /// </summary>
    public object Sender;

    /// <summary>
    /// 事件ID
    /// </summary>
    public SolarEventID ID;

    /// <summary>
    /// 事件数据
    /// </summary>
    public ISolarEventData Data;

    public SolarEvent(SolarEventID eventID, ISolarEventData eventData)
    {
        Sender = null;
        ID = eventID;
        Data = eventData;
    }

    public SolarEvent(object sender, SolarEventID eventID, ISolarEventData eventData)
    {
        Sender = sender;
        ID = eventID;
        Data = eventData;
    }

    public void Set(SolarEventID eventID, ISolarEventData eventData)
    {
        Sender = null;
        ID = eventID;
        Data = eventData;
    }

    public void Set(object sender, SolarEventID eventID, ISolarEventData eventData)
    {
        Sender = sender;
        ID = eventID;
        Data = eventData;
    }
}
