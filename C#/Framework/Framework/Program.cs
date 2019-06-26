namespace CSharp_Development_Kit
{
    using System;
    using Framework.DesignPattern;
 
    public class EventIDAttribute : Attribute
    {
        Type m_bindDataType;

        public EventIDAttribute(Type bindDataType)
        {
            if (bindDataType != typeof(IEventData)) return;

            m_bindDataType = bindDataType;
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
        }
    }
}