using Framework.DesignPattern;

/// <summary>
/// ObjectPool<FastString>
/// </summary>
public partial class FastString
{
    static readonly int s_DefaultPoolSize = 128;

    public static ObjectPool<FastString> s_Pool = new ObjectPool<FastString>(
        s_DefaultPoolSize,
        () => { return new FastString(); },
        (fastString) => { fastString.Clear(); }
    );

    /// <summary>
    /// Take a 'FastString' obj from pool
    /// </summary>
    /// <returns></returns>
    public static FastString Aquire()
    {
        return s_Pool?.Take();
    }

    /// <summary>
    /// Return 'FastString' obj to pool. if head is a linked list, return all the linked list node
    /// </summary>
    /// <param name="head"></param>
    public static void Release(FastString head)
    {
        var next = head;
        while (next)
        {
            var tempObj = next;
            var nextNode = next.m_nextNode;

            tempObj.m_nextNode = null;
            s_Pool?.Return(ref tempObj);

            next = nextNode;
        }
    }

    /// <summary>
    /// clear pool
    /// </summary>
    public static void ReleasePool()
    {
        s_Pool?.Clear();
        s_Pool = null;
    }
}