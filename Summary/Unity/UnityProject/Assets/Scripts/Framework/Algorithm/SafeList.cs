using System.Collections;
using System.Collections.Generic;

/// <summary>
/// SafeList
/// 
/// 相比源生的List, 支持在foreach遍历集合时, 动态添加删除元素
/// 
/// 采用两个额外的缓存列表用于添加删除元素, 外部每次访问数据列表时, 会自动刷新
/// 
/// </summary>
/// <typeparam name="T"></typeparam>
class SafeList<T> : IEnumerable<T>
{
    private const int s_DefaultSize = 4;

    // 数据列表
    List<T> m_Elements;

    // 添加元素的缓存列表
    List<T> m_AddCacheList;

    // 删除元素的缓存列表
    List<T> m_RemoveCacheList;

    public T this[int index]
    {
        get
        {
            Flush();
            return m_Elements[index];
        }
    }

    public int Count
    {
        get
        {
            Flush();
            return m_Elements.Count;
        }
    }

    public SafeList(int capacity = s_DefaultSize)
    {
        m_Elements = new List<T>(capacity);

        m_AddCacheList = null;
        m_RemoveCacheList = null;
    }

    public bool Contains(T item)
    {
        Flush();
        return m_Elements.Contains(item);
    }

    public void Add(T item)
    {
        if (m_AddCacheList == null)
            m_AddCacheList = new List<T>(s_DefaultSize);

        m_AddCacheList.Add(item);
    }

    public void Remove(T item)
    {
        if (m_RemoveCacheList == null)
            m_RemoveCacheList = new List<T>(s_DefaultSize);

        if (m_Elements.Contains(item))
            m_RemoveCacheList.Add(item);
    }

    public void Clear()
    {
        m_AddCacheList?.Clear();
        m_RemoveCacheList?.Clear();

        m_Elements?.Clear();
    }

    /// <summary>
    /// 刷新数据列表
    /// 1、将AddCacheList中的元素加入到DataList中
    /// 2、将RemoveCacheList中的元素从DataList中删除
    /// </summary>
    void Flush()
    {
        if (m_AddCacheList?.Count > 0)
        {
            foreach (var addItem in m_AddCacheList)
            {
                m_Elements.Add(addItem);
            }
            m_AddCacheList.Clear();
        }

        if (m_RemoveCacheList?.Count > 0)
        {
            foreach (var removeItem in m_RemoveCacheList)
            {
                m_Elements.Remove(removeItem);
            }
            m_RemoveCacheList.Clear();
        }
    }

    public IEnumerator<T> GetEnumerator()
    {
        Flush();
        return m_Elements.GetEnumerator();
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
        Flush();
        return m_Elements.GetEnumerator();
    }
}