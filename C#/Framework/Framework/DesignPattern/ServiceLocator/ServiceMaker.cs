namespace Framework.DesignPattern
{
    using System;
    using System.Reflection;

    static class ServiceMaker
    {
        /// <summary>
        /// 创建服务实例
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static T CreateService<T>() where T : class, ICommonService
        {
            ConstructorInfo[] ctorInfoArray = typeof(T).GetConstructors(BindingFlags.Instance | BindingFlags.NonPublic);

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
                throw new NotSupportedException("There is no non-public constructor with no params!");
            }

            T service = default(T);
            service = (T)zeroParamCtorInfo.Invoke(null);

            return service;
        }
    }
}