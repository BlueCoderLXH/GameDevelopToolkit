using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// 状态机
/// </summary>
/// <typeparam name="T"></typeparam>
public class StateMachine<T> where T : System.Enum
{
    /// <summary>
    /// 状态回调
    /// </summary>
    public delegate void StateFunc();

    /// <summary>
    /// 状态
    /// </summary>
    private class State
    {
        // 状态id
        public T Id;

        // 进入状态回调
        public StateFunc Enter;
        // 更新状态回调
        public StateFunc Update;
        // 离开状态回调
        public StateFunc Leave;

        public State(T id, StateFunc enter, StateFunc update, StateFunc leave)
        {
            Id = id;
            Enter = enter;
            Update = update;
            Leave = leave;
        }
    }

    // 当前状态 
    State m_CurrentState;

    // 状态集合
    Dictionary<T, State> m_States;

    public T CurrentState
    {
        get { return m_CurrentState.Id; }
    }

    public StateMachine()
    {
        m_States = new Dictionary<T, State>();
    }

    public void Add(T id, StateFunc enter, StateFunc update, StateFunc leave)
    {
        m_States.Add(id, new State(id, enter, update, leave));
    }

    public void Update()
    {
        m_CurrentState.Update();
    }

    public void SwitchTo(T state)
    {
        GameDebug.Assert(m_States.ContainsKey(state), "Trying to switch to unknown state " + state.ToString());
        GameDebug.Assert(m_CurrentState == null || !m_CurrentState.Id.Equals(state), "Trying to switch to " + state.ToString() + " but that is already current state");

        var newState = m_States[state];

        string msg = "Switching state: " + (m_CurrentState != null ? m_CurrentState.Id.ToString() : "null") + " -> " + state.ToString();
#if SERVER
            GameDebug.LogServer(msg);
#else
        GameDebug.LogClient(msg);
#endif

        if (m_CurrentState != null && m_CurrentState.Leave != null)
            m_CurrentState.Leave();

        if (newState.Enter != null)
            newState.Enter();

        m_CurrentState = newState;
    }

    public void Shutdown()
    {
        if (m_CurrentState != null && m_CurrentState.Leave != null)
            m_CurrentState.Leave();
        m_CurrentState = null;
    }
}