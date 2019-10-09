
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

        public void Assert(bool flag, string msg, TLabel label = default)
        {
            if (m_SwitchFlag) Debug.Assert(flag, $"[A] - [{label}] {msg}");
        }

        public void Log(string msg, TLabel label = default)
        {
            if (m_SwitchFlag) Debug.Log($"[L] - [{label}] {msg}");
        }

        public void Warn(string msg, TLabel label = default)
        {
            if (m_SwitchFlag) Debug.LogWarning($"[W] - [{label}] {msg}");
        }

        public void Error(string msg, TLabel label = default)
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

    public static void Assert(bool flag, string msg, DebugFlag label = default)
    {
        s_Debug.Assert(flag, msg, label);
    }

    public static void Log(string msg, DebugFlag label = default)
    {
        s_Debug.Log(msg, label);
    }

    public static void Warn(string msg, DebugFlag label = default)
    {
        s_Debug.Log(msg, label);
    }

    public static void Error(string msg, DebugFlag label = default)
    {
        s_Debug.Log(msg, label);
    }
}