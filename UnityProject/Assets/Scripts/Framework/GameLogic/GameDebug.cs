namespace Framework.Debug
{
    using UnityEngine;

    internal class GameDebug<TLabel>
        where TLabel : System.Enum
    {
        static string[] s_ColorStringArray = new string[] {
            "ffffffff", // White
            "000000ff", // Black
            "ff0000ff", // Red
            "1bf625ff", // Green
            "00d6fdff", // Blue
            "fdd617ff", // Yellow
            "f66c96ff"  // Pink
        };

        private bool m_SwitchFlag = true;

        public void Config(bool switchFlag)
        {
            m_SwitchFlag = switchFlag;
        }

        public void Assert(TLabel label, bool flag, FastString msg)
        {
            if (m_SwitchFlag && !flag) Debug.Assert(flag, $"<color=#ff0000ff>[A] - [{label}] {msg}</color>");
        }

        public void Log(TLabel label, FastString msg, DebugColor colorType = DebugColor.White)
        {
            if (m_SwitchFlag) Debug.Log($"<color=#{s_ColorStringArray[(int)colorType]}>[L] - [{label}] {msg}</color>");
        }

        public void Warn(TLabel label, FastString msg)
        {
            if (m_SwitchFlag) Debug.LogWarning($"<color=#fdd617ff>[W] - [{label}] {msg}</color>");
        }

        public void Error(TLabel label, FastString msg)
        {
            if (m_SwitchFlag) Debug.LogError($"<color=#ff0000ff>[E] - [{label}] {msg}</color>");
        }
    }
}

public enum DebugColor
{
    White,
    Black,
    Red,
    Green,
    Blue,
    Yellow,
    Pink
}

public enum DebugFlag
{
    Common
}

public static class GDebug
{
    static bool s_Flag;

    static Framework.Debug.GameDebug<DebugFlag> s_Debug;

    static GDebug()
    {
#if GameDebug
        s_Flag = true;
#else
        s_Flag = false;
#endif

        s_Debug = new Framework.Debug.GameDebug<DebugFlag>();
        s_Debug.Config(s_Flag);
    }

    public static void Assert(DebugFlag label, bool flag, FastString msg)
    {
        s_Debug.Assert(label, flag, msg);
        FastString.Release(msg);
    }

    public static void Log(DebugFlag label, FastString msg, DebugColor colorType = DebugColor.White)
    {
        s_Debug.Log(label, msg, colorType);
        FastString.Release(msg);
    }

    public static void Warn(DebugFlag label, FastString msg)
    {
        s_Debug.Warn(label, msg);
        FastString.Release(msg);
    }

    public static void Error(DebugFlag label, FastString msg)
    {
        s_Debug.Error(label, msg);
        FastString.Release(msg);
    }
}