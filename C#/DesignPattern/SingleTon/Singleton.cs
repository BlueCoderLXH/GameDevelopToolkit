using System;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// 单例基类
/// </summary>
public abstract class Singleton<T> where T : class
{
    protected static T _instance;

    public static T Instance
    {
        get
        {
            _instance = _instance ?? CreateInstance();
            return _instance;
        }
    }

    private static T CreateInstance()
    {
        ConstructorInfo[] ctorInfoArray = typeof(T).GetConstructors(BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public);

        ConstructorInfo zeroParamCtorInfo = null;

        foreach (ConstructorInfo ctorInfo in ctorInfoArray)
        {
            ParameterInfo[] ctorParams = ctorInfo.GetParameters();
            if (0 == ctorParams.Length)
            {
                zeroParamCtorInfo = ctorInfo;
                break;
            }
        }

        if (null == zeroParamCtorInfo)
        {
            throw new NotSupportedException("There is no non-param constructor!");
        }

        T instance = default(T);
        instance = (T)zeroParamCtorInfo.Invoke(null);

        return instance;
    }
}