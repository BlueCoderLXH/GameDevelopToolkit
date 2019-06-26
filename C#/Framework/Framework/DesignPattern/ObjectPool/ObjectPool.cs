using System;
using System.Collections;
using System.Collections.Generic;


namespace Framework.DesignPattern.ObjectPool
{
    using System;
    using System.Collections;
    using System.Collections.Generic;

    //using UnityEngine;

    /// <summary>
    /// 对象池
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class ObjectPool<T>
    {
        const int s_DefaultCapacity = 16;
        const int s_MinCapacity = 4;
        const int s_GrowFactor = 2;
        const int s_MinGrowSize = 4;

        private Func<T> m_ObjFactory;

        private Queue<T> m_ElementsQ;
        private int m_Capacity = 0;

        public ObjectPool(Func<T> factory) : this(s_DefaultCapacity, factory) { }

        public ObjectPool(int capacity, Func<T> factory)
        {
            //Debug.Assert(factory != null, "You should set a factory of type 'T'");

            capacity = Math.Max(s_MinCapacity, capacity);

            m_ObjFactory = factory;
            m_ElementsQ = new Queue<T>(capacity);

            Initialize(capacity);
        }

        private void Initialize(int capacity)
        {
            Expand(capacity);
        }

        private void Expand(int count)
        {
            //Debug.Assert(m_ElementsQ != null, "Pool cache is null!");

            int realExpandCount = Math.Max(count, s_MinGrowSize);

            for (int i = 0; i < realExpandCount; i++)
            {
                m_ElementsQ?.Enqueue(m_ObjFactory());
            }

            m_Capacity += count;

            //Debug.LogFormat("Increase:{0} Size:{1} Capacity:{2}", realIncreaseCount, m_ElementsQ.Count, m_Capacity);
        }

        private bool IsEmpty()
        {
            //Debug.Assert(m_ElementsQ != null, "Pool cache is null!");
            return m_ElementsQ.Count <= 0;
        }

        public T Take()
        {
            //Debug.Assert(m_ElementsQ != null, "Pool cache is null!");

            if (IsEmpty())
            {
                int expandCount = m_Capacity * s_GrowFactor / 2;
                Expand(expandCount);
            }

            return m_ElementsQ.Dequeue();
        }

        public void Return(T returnObj)
        {
            //Debug.Assert(m_ElementsQ != null, "Pool cache is null!");
            m_ElementsQ.Enqueue(returnObj);
        }

        public void Clear()
        {
            //Debug.Assert(m_ElementsQ != null, "Pool cache is null!");
            m_ElementsQ.Clear();
            m_ElementsQ = null;
        }
    }
}