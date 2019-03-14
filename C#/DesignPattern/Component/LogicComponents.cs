namespace Framework.LogicComponent
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// 游戏逻辑组件接口
    /// </summary>
    public interface ILogicComponent
    {
        //void OnInit();
        void OnUpdate(float dt);
        void OnDestroy();
    }

    /// <summary>
    /// 游戏逻辑组件集合
    /// 采用组件设计模式(Component Pattern)实现
    /// </summary>
    public class LogicComponents
    {
        Dictionary<Type, ILogicComponent> m_Components;

        /// <summary>
        /// 构造
        /// </summary>
        /// <param name="capacity">组件数</param>
        public LogicComponents(int capacity)
        {
            if (capacity <= 0)
            {
                capacity = 1;
            }

            m_Components = new Dictionary<Type, ILogicComponent>(capacity);
        }

        /// <summary>
        /// 添加组件
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
        /// 添加组件
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="component"></param>
        public void AddCompnent<T>(T component) where T : ILogicComponent
        {
            Type compType = component.GetType();

            if (!m_Components.ContainsKey(compType))
            {
                m_Components.Add(compType, component);
            }
        }

        /// <summary>
        /// 获取组件
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
        /// 更新所有组件
        /// </summary>
        /// <param name="dt"></param>
        public void Update(float dt)
        {
            foreach (var component in m_Components)
            {
                component.Value.OnUpdate(dt);
            }
        }

        /// <summary>
        /// 释放所有的组件
        /// </summary>
        public void Destroy()
        {
            foreach (var component in m_Components)
            {
                component.Value.OnDestroy();
            }
        }
    }
}