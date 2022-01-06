using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Algorithm.QuadTree
{
    /// <summary>
    /// 子节点类型
    /// </summary>
    public enum QuadTreeAreaType
    {
        None = -1,
        LeftUp,
        RightUp,
        RightBottom,
        LeftBottom
    }

    /// <summary>
    /// 四叉树节点的坐标信息
    /// </summary>
    public struct CoordInfo
    {
        // 节点中心点
        public Vector2 Center;

        // 节点坐标范围
        public Vector2 Range;

        public CoordInfo(CoordInfo fatherRange, QuadTreeAreaType childNodeType)
        {
            Center = fatherRange.Center;
            Range = fatherRange.Range / 2;

            Init(childNodeType);
        }

        private void Init(QuadTreeAreaType childNodeType)
        {
            switch (childNodeType)
            {
                case QuadTreeAreaType.LeftBottom:
                    Center.x -= Range.x / 2;
                    Center.y -= Range.y / 2;
                    break;

                case QuadTreeAreaType.LeftUp:
                    Center.x -= Range.x / 2;
                    Center.y += Range.y / 2;
                    break;

                case QuadTreeAreaType.RightUp:
                    Center.x += Range.x / 2;
                    Center.y += Range.y / 2;
                    break;

                case QuadTreeAreaType.RightBottom:
                    Center.x += Range.x / 2;
                    Center.y -= Range.y / 2;
                    break;
            }
        }

        public QuadTreeAreaType Locate(Vector2 targetPos)
        {
            bool inRange = (targetPos.x >= (Center.x - Range.x / 2) && targetPos.x <= (Center.x + Range.x / 2)) &&
                (targetPos.y >= (Center.y - Range.y / 2) && targetPos.y <= (Center.y + Range.y / 2));

            QuadTreeAreaType ret;
            if (inRange)
            {
                if (targetPos.x < Center.x)
                {
                    ret = targetPos.y >= Center.y ? QuadTreeAreaType.LeftUp : QuadTreeAreaType.LeftBottom;
                }
                else
                {
                    ret = targetPos.y >= Center.y ? QuadTreeAreaType.RightUp : QuadTreeAreaType.RightBottom;
                }
            }
            else
            {
                ret = QuadTreeAreaType.None;
            }

            return ret;
        }

        public override string ToString()
        {
            return string.Format("pos[{0}, {1}], size[{2}, {3}]", Center.x, Center.y, Range.x, Range.y);
        }
    }

    /// <summary>
    /// 四叉树节点
    /// </summary>
    public class QuadTreeNode
    {
        protected QuadTreeAreaType m_AreaType;

        protected List<QuadTreeNode> m_Childs;

        protected CoordInfo m_CoordRange;

        protected uint m_Layer;

        public bool IsLeaf { get { return m_Childs == null || m_Childs.Count == 0; } }

        public QuadTreeNode(QuadTreeAreaType areaType, CoordInfo coordRange, uint layer, bool leaf = false)
        {
            if (!leaf)
            {
                m_Childs = new List<QuadTreeNode>(QuadTreeConfig.ChildCount);
                for (int i = 0; i < QuadTreeConfig.ChildCount; i++)
                {
                    m_Childs.Add(null);
                }
            }

            m_AreaType = areaType;
            m_CoordRange = coordRange;
            m_Layer = layer;
        }

        public QuadTreeNode InsertChild(QuadTreeAreaType childNodeType)
        {
            if (IsLeaf) return null;

            CoordInfo childCoordRange = new CoordInfo(m_CoordRange, childNodeType);
            uint childLayer = m_Layer + 1;

            QuadTreeNode childNode;
            if (childLayer < QuadTreeConfig.MaxLayer)
            {
                childNode = new QuadTreeNode(childNodeType, childCoordRange, childLayer);
            }
            else
            {
                childNode = new QuadTreeLeafNode(childNodeType, childCoordRange);
            }

            m_Childs[(int)childNodeType] = childNode;

            return childNode;
        }

        public QuadTreeAreaType Locate(Vector3 targetPos)
        {
            return m_CoordRange.Locate(new Vector2(targetPos.x, targetPos.z));
        }

        public bool HandleObject(Vector3 pos, bool isAdd)
        {
            QuadTreeNode locateNode = Search(pos);
            if (locateNode == null) return false;

            locateNode.OnHandleObject(pos, isAdd);
            return true;
        }

        private QuadTreeNode Search(Vector3 targetPos)
        {
            QuadTreeAreaType areaType = Locate(targetPos);
            if (areaType == QuadTreeAreaType.None)
            {
                return null;
            }

            var nextChild = m_Childs[(int)areaType];
            if (!nextChild.IsLeaf)
            {
                return nextChild.Search(targetPos);
            }

            return nextChild;
        }

        protected virtual void OnHandleObject(Vector3 pos, bool isAdd) { }

        public override string ToString()
        {
            return m_CoordRange.ToString();
        }
    }
}