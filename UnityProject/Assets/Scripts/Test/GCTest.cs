using System;
using System.Text;
using Framework.DesignPattern;
using UnityEngine;

public class GCTest : ITestCase
{
    const int Times = 50;

    string m_str = "";
    StringBuilder m_sb = new StringBuilder("123456789");
    FastString m_fs = FastString.Aquire();

    void ITestCase.Init()
    {
        GDebug.Assert(DebugFlag.Common, false, "Assert");

        GDebug.Log(DebugFlag.Common, "Log", DebugColor.White);
        GDebug.Log(DebugFlag.Common, "Log", DebugColor.Black);
        GDebug.Log(DebugFlag.Common, "Log", DebugColor.Red);
        GDebug.Log(DebugFlag.Common, "Log", DebugColor.Green);
        GDebug.Log(DebugFlag.Common, "Log", DebugColor.Blue);
        GDebug.Log(DebugFlag.Common, "Log", DebugColor.Yellow);
        GDebug.Log(DebugFlag.Common, "Log", DebugColor.Pink);

        GDebug.Warn(DebugFlag.Common, "Warn");
        GDebug.Error(DebugFlag.Common, "Error");
    }

    void ITestCase.Update()
    {
        //TestStringOperation();
        //TestLog();
    }

    void ITestCase.Release()
    {
        FastString.Release(m_fs);
        FastString.ReleasePool();
    }

    void TestStringOperation()
    {
        NewString();

        NewStringBuilder();

        NewStringFast();
    }

    void TestLog()
    {
        NewStringFormat();
        NewStringFormatFastString();
    }

    void NewString()
    {
        for (int i = 0; i < Times; i++)
        {
            m_str = 123.ToString() + 456.ToString() + 789.ToString();
            m_str.Replace("123456789", "123456789");
            //m_str.ToString();
        }
    }

    void NewStringBuilder()
    {
        for (int i = 0; i < Times; i++)
        {
            m_sb.Clear();

            m_sb.Append("123");
            m_sb.Append("456");
            m_sb.Append("789");

            m_sb.Replace("123456789", "123456789");

            //m_sb.ToString();
        }
    }

    void NewStringFast()
    {
        for (int i = 0; i < Times; i++)
        {
            //m_fs.Replace("123456789", "123456789");
            //m_fs.Replace("1", "1");
            //m_fs.ToString();

            m_fs.Clear();

            m_fs.Append(123).Append(456).Append(789);
            m_fs.Replace("123456789", "123456789");
        }
    }

    void NewStringFormat()
    {
        for (int i = 0; i < Times; i++)
        {
            GDebug.Log(DebugFlag.Common, $"{123} {456} {789}");
        }
    }

    void NewStringFormatFastString()
    {
        for (int i = 0; i < Times; i++)
        {
            GDebug.Log(DebugFlag.Common, FastString.Format("[F]{0} {1} {2}", 123, 456, 789));
        }
    }
}