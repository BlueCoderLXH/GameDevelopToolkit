namespace Framework.DesignPattern
{
    using System;
    using System.Collections.Generic;

    public interface ILogicComponent
    {
        void OnInit();
        void OnUpdate(float dt);
        void OnDestroy();
    }

    /// <summary>
    /// [LogicComponents]
    /// 
    /// 采用组件模式(Component Pattern)实现, 包含继承'ILogicComponent'的逻辑对象集合
    /// </summary>
    public class LogicComponents
    {
        Dictionary<Type, ILogicComponent> m_Components;

        /// <summary>
        /// 构造
        /// </summary>
        /// <param name="capacity">预先缓存的组件个数</param>
        public LogicComponents(int capacity)
        {
            if (capacity <= 0)
            {
                capacity = 1;
            }

            m_Components = new Dictionary<Type, ILogicComponent>(capacity);
        }

        /// <summary>
        /// 添加一个指定T类型的组件对象(自动初始化)
        /// </summary>
        /// <typeparam name="T"></typeparam>
        public void AddComponent<T>() where T : ILogicComponent, new()
        {
            Type compType = typeof(T);

            if (!m_Components.ContainsKey(compType))
            {
                T component = new T();

                m_Components.Add(compType, component);
            }
        }

        /// <summary>
        /// 添加一个指定T类型且已经初始化的组件对象
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="component"></param>
        public void AddComponent<T>(T component) where T : ILogicComponent
        {
            Type compType = component.GetType();

            if (!m_Components.ContainsKey(compType))
            {
                m_Components.Add(compType, component);
            }
        }

        /// <summary>
        /// 获取组件对象
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public T GetComponent<T>() where T : class, ILogicComponent
        {
            Type compType = typeof(T);

            ILogicComponent component = null;

            if (!m_Components.TryGetValue(compType, out component))
            {
                return default(T);
            }

            return component as T;
        }

        /// <summary>
        /// 初始化所有的组件
        /// </summary>
        public void InitAll()
        {
            foreach (var component in m_Components)
            {
                component.Value.OnInit();
            }
        }

        /// <summary>
        /// 更新所有的组件
        /// </summary>
        /// <param name="dt"></param>
        public void UpdateAll(float dt)
        {
            foreach (var component in m_Components)
            {
                component.Value.OnUpdate(dt);
            }
        }

        /// <summary>
        /// 销毁所有的组件
        /// </summary>
        public void DestroyAll()
        {
            foreach (var component in m_Components)
            {
                component.Value.OnDestroy();
            }
        }
    }
}