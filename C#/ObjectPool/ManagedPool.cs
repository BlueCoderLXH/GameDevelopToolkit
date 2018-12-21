using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Poolable
{
    protected int m_Identifier = 0;

    public int Identifier
    {
        get { return m_Identifier; }
    }

    public Poolable(int identifier)
    {
        m_Identifier = identifier;
    }
}

public class ManagedPool<T> where T : Poolable
{
    /// <summary>
    /// 对象池
    /// </summary>
    protected Dictionary<int, Stack<T>> m_Pool = null;

    protected ManagedPool()
    {
        m_Pool = new Dictionary<int, Stack<T>>();
    }
    
    public void Push(T instance)
    {
        Stack<T> instances = null;
        if (m_Pool.TryGetValue(instance.Identifier, out instances))
        {
            instances.Push(instance);
        }
        else
        {
            instances = new Stack<T>();
            instances.Push(instance);
            //
            m_Pool.Add(instance.Identifier, instances);
        }
    }

    public T Pop(int id)
    {
        T result = null;
        Stack<T> instances = null;
        if (m_Pool.TryGetValue(id, out instances))
        {
            if (instances.Count > 0)
            {
                result = instances.Pop();
            }
        }
        return result;
    }
}