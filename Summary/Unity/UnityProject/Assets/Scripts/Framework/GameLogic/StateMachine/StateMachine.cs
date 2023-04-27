namespace Framework.StateMachine
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// 状态机
    /// 
    /// 特点:
    ///   1、状态ID支持模板
    ///   2、无GC开销
    /// </summary>
    public class StateMachine<T>
        where T : Enum
    {
        T m_CurrentStateId;

        // 当前状态 
        State m_CurrentState;

        // 状态集合 [StateId, State]
        Dictionary<int, State> m_States;

        public T CurrentState
        {
            get { return m_CurrentStateId; }
        }

        public StateMachine(int stateCount = 4)
        {
            m_CurrentStateId = default;
            m_CurrentState = State.Null;

            m_States = new Dictionary<int, State>(stateCount);
        }

        public void Add(T stateId, StateFunc onEnter = null, StateFunc onUpdate = null, StateFunc onLeave = null)
        {
            // 枚举类型内部是整型, int的hashcode就是其本身, 因此枚举值的hashcode就是其对应的整型值
            int int_id = stateId.GetHashCode();
            m_States.Add(int_id, new State(int_id, onEnter, onUpdate, onLeave));
        }

        public void Update()
        {
            m_CurrentState.OnUpdate();
        }

        public void SwitchTo(T stateId)
        {
            int int_StateId = stateId.GetHashCode();

            State nextState;
            if (!m_States.TryGetValue(int_StateId, out nextState))
            {
                //GDebug.Assert(false, $"Trying to switch to unknown state '{stateId}'");
            }

            if (m_CurrentState == nextState)
            {
                //GDebug.Assert(false, $"Trying to switch to '{stateId}' but that is already current state");
            }

            //GDebug.Log($"Switching state: {m_CurrentStateId} -> {stateId}");

            m_CurrentState.OnLeave();

            nextState.OnEnter();

            m_CurrentState = nextState;
            m_CurrentStateId = stateId;
        }

        public void Shutdown()
        {
            m_CurrentState.OnLeave();
            m_CurrentState = State.Null;
        }
    }
}