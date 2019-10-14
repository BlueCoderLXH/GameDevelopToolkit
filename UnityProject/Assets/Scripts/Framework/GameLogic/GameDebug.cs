
namespace Framework.Debug
{
    using UnityEngine;

    internal class GameDebug<TLabel>
        where TLabel : System.Enum
    {
        private bool m_SwitchFlag = true;

        public void Config(bool switchFlag)
        {
            m_SwitchFlag = switchFlag;
        }

        public void Assert(bool flag, FastString msg, TLabel label = default)
        {
            if (m_SwitchFlag && !flag) Debug.Assert(flag, $"[A] - [{label}] {msg}");
        }

        public void Log(FastString msg, TLabel label = default)
        {
            if (m_SwitchFlag) Debug.Log($"[L] - [{label}] {msg}");
        }

        public void Warn(FastString msg, TLabel label = default)
        {
            if (m_SwitchFlag) Debug.LogWarning($"[W] - [{label}] {msg}");
        }

        public void Error(FastString msg, TLabel label = default)
        {
            if (m_SwitchFlag) Debug.LogError($"[E] - [{label}] {msg}");
        }
    }
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

    public static void Assert(bool flag, FastString msg, DebugFlag label = default)
    {
        s_Debug.Assert(flag, msg, label);
        FastString.Release(msg);
    }

    public static void Log(FastString msg, DebugFlag label = default)
    {
        s_Debug.Log(msg, label);
        FastString.Release(msg);
    }

    public static void Warn(FastString msg, DebugFlag label = default)
    {
        s_Debug.Log(msg, label);
        FastString.Release(msg);
    }

    public static void Error(FastString msg, DebugFlag label = default)
    {
        s_Debug.Log(msg, label);
        FastString.Release(msg);
    }
}