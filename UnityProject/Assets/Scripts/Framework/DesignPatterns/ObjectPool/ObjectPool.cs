namespace Framework.DesignPattern
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// 可重复利用的对象
    /// 
    /// 使用ReusableObject对T类型对象包装, 对GC更加友好
    /// 
    /// 注:对于一个类型为X的数组array,
    /// => 当X为引用类型时, GC除了需要判断array是否为空外, 还需要遍历判断所有的数组元素array[i]是否为空
    /// => 当X为值类型时, GC只需要判断array是否为空即可, 因为值类型不可能为空
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public struct ReusableObject<T> where T : class
    {
        private T m_CacheObj;

        public T CacheObj
        {
            get { return m_CacheObj; }
        }

        public ReusableObject(T cacheObj)
        {
            m_CacheObj = cacheObj;
        }
    }

    /// <summary>
    /// 类型T的对象池
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class ObjectPool<T> where T : class
    {
        // 对象池默认容量
        const int s_DefaultCapacity = 16;
        // 对象池最小容量
        const int s_MinCapacity = 4;
        // 对象池增长倍率
        const int s_GrowFactor = 2;
        // 最小增长数
        const int s_MinGrowSize = 4;

        // T类型的对象工厂方法(用于生成T类型的对象)
        private Func<T> m_ObjFactory;

        // T类型对象重置方法
        private Action<T> m_ObjResetFunc;

        // 对象池([Hashcode of type 'T' object, ReusableObject<T>])
        private Dictionary<int, ReusableObject<T>> m_BufferPool;

        // 可使用的对象key值集合
        private List<int> m_UsableObjKeys;

        // 当前对象池的容量
        private int m_Capacity = 0;

        /// <summary>
        /// 创建一个T类型的对象池, 默认大小
        /// </summary>
        /// <param name="factory"></param>
        public ObjectPool(Func<T> factory, Action<T> objResetFunc = null) :
            this(s_DefaultCapacity, factory, objResetFunc)
        { }

        /// <summary>
        /// 创建一个T类型的对象池, 指定大小
        /// </summary>
        /// <param name="capacity"></param>
        /// <param name="factory"></param>
        public ObjectPool(int capacity, Func<T> factory, Action<T> objResetFunc = null)
        {
            GDebug.Assert(factory != null, "You should set a factory of type 'T' to create 'T'-type objects");

            capacity = Math.Max(s_MinCapacity, capacity);

            m_ObjFactory = factory;
            m_ObjResetFunc = objResetFunc;

            m_BufferPool = new Dictionary<int, ReusableObject<T>>(capacity);
            m_UsableObjKeys = new List<int>(capacity);

            Initialize(capacity);
        }

        /// <summary>
        /// 初始化对象池
        /// </summary>
        /// <param name="capacity"></param>
        private void Initialize(int capacity)
        {
            Expand(capacity);
        }

        /// <summary>
        /// 对象池扩容
        /// </summary>
        /// <param name="count"></param>
        private void Expand(int count)
        {
            GDebug.Assert(m_BufferPool != null, "Pool cache is null!");

            m_Capacity += count;

            int realExpandCount = Math.Max(count, s_MinGrowSize);

            for (int i = 0; i < realExpandCount; i++)
            {
                T newObj = m_ObjFactory();
                var usableObj = new ReusableObject<T>(newObj);

                int objKey = newObj.GetHashCode();

                GDebug.Assert(!m_BufferPool.ContainsKey(objKey), "Can't add the same obj to buffer pool!");

                m_BufferPool.Add(objKey, usableObj);
                m_UsableObjKeys.Add(objKey);
            }
        }

        /// <summary>
        /// 对象池是否为空
        /// </summary>
        /// <returns></returns>
        private bool IsEmpty()
        {
            GDebug.Assert(m_BufferPool != null, "Pool cache is null!");
            return m_UsableObjKeys.Count <= 0;
        }

        /// <summary>
        /// 从对象池中取一个可用的对象
        /// </summary>
        /// <returns></returns>
        public T Take()
        {
            GDebug.Assert(m_BufferPool != null, "Pool cache is null!");

            if (IsEmpty())
            {
                int expandCount = m_Capacity * (s_GrowFactor - 1);
                Expand(expandCount);
            }

            int takeObjIndex = m_UsableObjKeys.Count - 1;
            int takedObjKey = m_UsableObjKeys[takeObjIndex];
            var takedObj = m_BufferPool[takedObjKey];

            m_UsableObjKeys.RemoveAt(takeObjIndex);

            return takedObj.CacheObj;
        }

        /// <summary>
        /// 将对象返回对象池
        /// </summary>
        /// <param name="returnObj"></param>
        public void Return(ref T returnObj)
        {
            GDebug.Assert(m_BufferPool != null, "Pool cache is null!");

            int returnObjKey = returnObj.GetHashCode();
            if (m_BufferPool.ContainsKey(returnObjKey))
            {
                if (m_ObjResetFunc != null)
                {
                    m_ObjResetFunc(returnObj);
                }

                m_UsableObjKeys.Add(returnObjKey);

                returnObj = null;
            }
        }

        /// <summary>
        /// 清空对象池
        /// </summary>
        public void Clear()
        {
            GDebug.Assert(m_BufferPool != null, "Pool cache is null!");

            m_BufferPool.Clear();
            m_BufferPool = null;

            m_UsableObjKeys.Clear();
            m_UsableObjKeys = null;
        }
    }
}