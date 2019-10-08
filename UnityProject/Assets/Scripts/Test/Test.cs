using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// 测试用例接口
/// </summary>
public interface ITestCase
{
    void Init();
    void Update();
    void Release();
}

/// <summary>
/// 测试用例运行入口
/// </summary>
public class Test : MonoBehaviour
{
    List<ITestCase> m_TestCases = new List<ITestCase>();

    void Awake()
    {
        EventTest eTest = new EventTest();
        m_TestCases.Add(eTest);
    }

    void Start()
    {
        foreach(var testCase in m_TestCases)
        {
            testCase.Init();
        }
    }

    void OnDestroy()
    {
        foreach (var testCase in m_TestCases)
        {
            testCase.Release();
        }
    }

    void Update()
    {
        foreach (var testCase in m_TestCases)
        {
            testCase.Update();
        }
    }
}