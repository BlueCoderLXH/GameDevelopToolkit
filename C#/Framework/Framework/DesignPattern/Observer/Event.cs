namespace Framework.DesignPattern
{
    /// <summary>
    /// 事件枚举
    /// 
    /// 注: 每新加一个新事件, 要按照规范写相关注释, 方便其他人能很快了解并正确使用该事件
    /// </summary>
    public enum EventID
    {
        /// <summary>
        /// 游戏结束事件 - no param
        /// </summary>
        GameOver,

        /// <summary>
        /// 游戏天数变化 - param1<int>: 天
        /// </summary>
        GameDaysChanged,
    }
}