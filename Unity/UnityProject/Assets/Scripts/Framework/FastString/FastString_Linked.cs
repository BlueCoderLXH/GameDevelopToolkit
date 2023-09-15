using System.Text;

/// <summary>
/// Concat FastString with a linked list
/// </summary>
public partial class FastString
{
    /// <summary>
    /// Linked node
    /// </summary>
    FastString m_nextNode;

    /// <summary>
    /// Append linked node
    /// </summary>
    /// <param name="nextNode"></param>
    /// <returns></returns>
    public FastString Append(FastString nextNode)
    {
        m_nextNode = nextNode;
        return m_nextNode;
    }

    /// <summary>
    /// Make linked string
    /// </summary>
    /// <returns></returns>
    private string ToLinkedString()
    {
        StringBuilder retStr = new StringBuilder();

        FastString next = this;
        while (next)
        {
            retStr.Append(next.ToNormalString());
            next = next.m_nextNode;
        }

        return retStr.ToString();
    }

    /// <summary>
    /// Make a string
    /// (Linked or Normal)
    /// </summary>
    /// <returns></returns>
    public override string ToString()
    {
        string ret = "";

        if (!m_nextNode)
            ret = ToNormalString();
        else
            ret = ToLinkedString();

        return ret;
    }
}