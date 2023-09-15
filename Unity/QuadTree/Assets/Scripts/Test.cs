using System.Text;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Profiling;

public class Test : MonoBehaviour
{
    StringBuilder m_String = new StringBuilder();

    void Awake()
    {
    }

    private void Update()
    {
        Profiler.BeginSample("Test:Update");
        m_String.Append("a");
        Profiler.EndSample();
    }
}