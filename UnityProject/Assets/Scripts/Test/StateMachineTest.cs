using Framework.StateMachine;

/// <summary>
/// 状态机测试
/// </summary>
public class StateMachineTest : ITestCase
{
    public enum StateFlag
    {
        Null,
        Browsing,
        Connecting,
        Connected,
        Shutdown
    }

    StateMachine<StateFlag> m_StateMachine = new StateMachine<StateFlag>();

    void ITestCase.Init()
    {
        m_StateMachine.Add(StateFlag.Browsing, OnBrowsing);
        m_StateMachine.Add(StateFlag.Connecting, OnConnecting);
        m_StateMachine.Add(StateFlag.Connected, OnConnected);
        m_StateMachine.Add(StateFlag.Shutdown, OnShutdown);

        m_StateMachine.SwitchTo(StateFlag.Browsing);
    }

    void OnBrowsing()
    {
        //GDebug.Log("OnBrowsing");
    }

    void OnConnecting()
    {
        //GDebug.Log("OnConnecting");
    }

    void OnConnected()
    {
        //GDebug.Log("OnConnected");
    }

    void OnShutdown()
    {
        //GDebug.Log("OnShutdown");
    }

    void ITestCase.Update()
    {
        switch (m_StateMachine.CurrentState)
        {
            case StateFlag.Browsing:
                m_StateMachine.SwitchTo(StateFlag.Connecting);
                break;

            case StateFlag.Connecting:
                m_StateMachine.SwitchTo(StateFlag.Connected);
                break;

            case StateFlag.Connected:
                m_StateMachine.SwitchTo(StateFlag.Shutdown);
                break;

            case StateFlag.Shutdown:
                break;
        }

        m_StateMachine.Update();
    }

    void ITestCase.Release()
    {
        m_StateMachine.Shutdown();
    }
}