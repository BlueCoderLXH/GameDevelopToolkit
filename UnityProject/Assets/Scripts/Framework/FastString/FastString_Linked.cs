using System.Text;

public partial class FastString
{
    FastString m_nextNode;

    public FastString Append(FastString nextNode)
    {
        m_nextNode = nextNode;
        return m_nextNode;
    }

    public string ToLinkedString()
    {
        if (m_nextNode == null)
        {
            return ToString();
        }

        StringBuilder retStr = new StringBuilder();
        retStr.Append(ToString());

        FastString next = m_nextNode;
        while (!next.IsEmpty())
        {
            retStr.Append(next);
            next = next.m_nextNode;
        }

        return retStr.ToString();
    }
}