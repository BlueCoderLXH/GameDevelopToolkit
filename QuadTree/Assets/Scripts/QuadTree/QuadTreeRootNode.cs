using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Algorithm.QuadTree
{
    /// <summary>
    /// 四叉树根节点
    /// </summary>
    public class QuadTreeRootNode : QuadTreeNode
    {
        public QuadTreeRootNode(CoordInfo coordRange)
            : base(QuadTreeAreaType.None, coordRange, QuadTreeConfig.RootLayer)
        {
        }
    }
}