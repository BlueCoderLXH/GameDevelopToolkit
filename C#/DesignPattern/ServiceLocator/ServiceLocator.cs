using System;
using System.Collections.Generic;

namespace Framework.Service
{
    /// <summary>
    /// ServiceLocator的抽象接口
    /// </summary>
    public interface ICommonService
    {
        void Release();
    }

    /// <summary>
    /// 服务定位器
    /// 1、凡是全局用到的服务, 都可以通过注入到ServiceLocator中, 减少项目中单例的个数
    /// 2、服务不能重复注入
    /// 3、ServiceLocator作为通用的框架代码, 不能直接使用, 使用需继承其之
    /// </summary>
    public abstract class ServiceLocator
    {
        private Dictionary<Type, ICommonService> m_Services;

        public ServiceLocator()
        {
            m_Services = new Dictionary<Type, ICommonService>();
        }

        private ICommonService Get(Type serviceType)
        {
            ICommonService service = null;

            if (!m_Services.TryGetValue(serviceType, out service))
            {
                return null;
            }

            return service;
        }

        /// <summary>
        /// 获取服务
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public T Get<T>() where T : class, ICommonService
        {
            ICommonService service = Get(typeof(T));

            if (service == null)
            {
                SolarLogger.LogWarningFormat(eOutPutModule.General, "ServiceLocator type '{0}' is not found!", typeof(T));
                return null;
            }

            return service as T;
        }

        /// <summary>
        /// 注入服务
        /// </summary>
        /// <param name="registerService"></param>
        public void Register<T>() where T : class, ICommonService
        {
            Type serviceType = typeof(T);

            if (Get(serviceType) == null)
            {
                ICommonService registerService = ServiceMaker.CreateService<T>();

                m_Services.Add(serviceType, registerService);
            }
        }

        /// <summary>
        /// 注入服务对象
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="service"></param>
        public void Register<T>(T service) where T : class, ICommonService
        {
            Type serviceType = service.GetType();

            if (Get(serviceType) == null)
            {
                m_Services.Add(serviceType, service);
            }
        }

        /// <summary>
        /// 注销服务
        /// </summary>
        /// <param name="registerService"></param>
        public void Unregister<T>() where T : ICommonService
        {
            Type serviceType = typeof(T);

            ICommonService service = Get(serviceType);

            if (service != null)
            {
                service.Release();
                m_Services.Remove(serviceType);
            }
        }

        /// <summary>
        /// 释放所有的服务对象
        /// (退出游戏时调用)
        /// </summary>
        public void Release()
        {
            foreach (var service in m_Services)
            {
                service.Value.Release();
            }

            m_Services.Clear();
        }
    }
}