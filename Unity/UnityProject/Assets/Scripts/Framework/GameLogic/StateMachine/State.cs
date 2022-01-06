namespace Framework.StateMachine
{
    /// <summary>
    /// 状态回调
    /// </summary>
    public delegate void StateFunc();

    /// <summary>
    /// 状态
    /// </summary>
    internal struct State
    {
        private const int NullId = -1000000;

        public static readonly State Null = new State(NullId);

        private int m_Id;

        private StateFunc m_OnEnter;
        private StateFunc m_OnUpdate;
        private StateFunc m_OnLeave;

        // 状态id
        public int Id => m_Id;

        public bool IsNull => Id == NullId;

        public State(int id, StateFunc enter = null, StateFunc update = null, StateFunc leave = null)
        {
            m_Id = id;

            m_OnEnter = enter;
            m_OnUpdate = update;
            m_OnLeave = leave;
        }

        public void OnEnter()
        {
            m_OnEnter?.Invoke();
        }

        public void OnUpdate()
        {
            m_OnUpdate?.Invoke();
        }

        public void OnLeave()
        {
            m_OnLeave?.Invoke();
        }

        public override string ToString()
        {
            return IsNull ? "Null" : Id.ToString();
        }

        public static bool operator ==(State a, State b)
        {
            return a.Id == b.Id;
        }

        public static bool operator !=(State a, State b)
        {
            return !(a == b);
        }
    }
}