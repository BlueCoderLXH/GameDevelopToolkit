using System;
using System.Text;
using Framework.DesignPattern;

public class GCTest : ITestCase
{
    const int Times = 50;

    string m_str = "123456789";
    StringBuilder m_sb = new StringBuilder("123456789");
    FastString m_fs = new FastString(10);

    void ITestCase.Init()
    {
        m_fs.Append(123456789);
    }

    void NewString()
    {
        for (int i = 0; i < Times; i++)
        {
            //m_str.Replace("123456789", "123456789");
            m_str.ToString();
        }
    }

    void NewStringBuilder()
    {
        for (int i = 0; i < Times; i++)
        {
            //m_sb.Replace("123456789", "123456789");
            m_sb.ToString();
        }
    }

    void NewStringFast()
    {
        for (int i = 0; i < Times; i++)
        {
            //m_fs.Replace("123456789", "123456789");
            //m_fs.Replace("1", "1");
            //m_fs.ToString();

            FastString.Format("123", "456", "789");
            //FastString.Pool.Return(ref str);
        }
    }

    void ITestCase.Update()
    {
        NewString();

        NewStringBuilder();

        NewStringFast();
    }

    void ITestCase.Release()
    {
    }
}