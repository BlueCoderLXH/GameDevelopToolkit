namespace Framework.DesignPattern.Command
{
    /// <summary>
    /// 命令基类
    /// </summary>
    public abstract class Command
    {
        private bool m_IsExcuted = false;
        public bool IsExcuted { get { return m_IsExcuted; } }

        /// <summary>
        /// 执行命令
        /// </summary>
        /// <param name="actor"></param>
        public void Execute(GameActor actor)
        {
            if (actor != null && !m_IsExcuted)
            {
                m_IsExcuted = OnExecute(actor);
            }
        }

        /// <summary>
        /// 撤销命令
        /// 1、可选择行为
        /// 2、命令执行前的状态需在子类中自行保存以撤销命令
        /// </summary>
        public void Undo()
        {
            if ( m_IsExcuted)
            {
                OnUndo();
            }
        }

        protected abstract bool OnExecute(GameActor actor);
        protected virtual void OnUndo() { }
    }

    /// <summary>
    /// 游戏角色相关的命令基类
    /// </summary>
    public abstract class GameActorCommand : Command
    {
        protected GameActor m_gameActor;

        protected override bool OnExecute(GameActor actor)
        {
            if (actor != null)
            {
                m_gameActor = actor;
            }

            return true;
        }

        protected override void OnUndo()
        {
            if (m_gameActor != null)
            {
                m_gameActor = null;
            }
        }
    }

    #region Command Example Code
    public class GameActor
    {
        public float X { get; protected set; }
        public float Y { get; protected set; }

        public void Move(float x, float y) { X = x; Y = y; }
    }

    public class MoveCommand : GameActorCommand
    {
        private float m_X;
        private float m_Y;
        private float m_BeforeX;
        private float m_BeforeY;

        public MoveCommand(float x, float y)
        {
            m_X = x;
            m_Y = y;
        }

        protected override bool OnExecute(GameActor gameActor)
        {
            base.OnExecute(gameActor);

            m_BeforeX = gameActor.X;
            m_BeforeY = gameActor.Y;
            gameActor.Move(m_X, m_Y);

            return true;
        }

        protected override void OnUndo()
        {
            m_gameActor.Move(m_BeforeX, m_BeforeY);

            base.Undo();
        }
    }
    #endregion
}