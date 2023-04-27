namespace Framework.DesignPattern
{
    using System;
    using System.Reflection;

    /// <summary>
    /// 单例基类
    /// 
    /// 单例模式的特点有两点：
    /// 1> 全局只有一个该类的实例对象
    /// 2> 全局都需要访问该单例对象
    /// 
    ///     如果某个类的特点符合上述两点, 可以采用单例模式, 但是同一个项目内单例类不应该太多
    /// 如果确实需要很多单例类, 考虑采用ServiceLocator对单例对象进行统一的管理
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
}