using Framework.DesignPattern;

/// <summary>
/// 事件系统测试用例
/// </summary>
public class EventTest : ITestCase
{
    EventListener<TestEvent1Data> m_EventListener;

    void ITestCase.Init()
    {
        m_EventListener = new EventListener<TestEvent1Data>(OnTestEvent);

        GEventDispatcher.Register(GEventID.TestEvent1, m_EventListener);
    }

    void ITestCase.Release()
    {
        GEventDispatcher.Unregister(GEventID.TestEvent1, m_EventListener);
    }

    void ITestCase.Update()
    {
        GEventDispatcher.Dispatch(GEventID.TestEvent1, new TestEvent1Data() { Name = "John", Age = 18 });
    }

    void OnTestEvent(TestEvent1Data eventData)
    {
    }
}
