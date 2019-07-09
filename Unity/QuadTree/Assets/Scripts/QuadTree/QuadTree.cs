using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Algorithm.QuadTree
{
    /// <summary>
    /// 四叉树
    /// </summary>
    public class QuadTree
    {
        QuadTreeRootNode m_Root;

        public QuadTree(Vector3 center, int terrainSize)
        {
            CoordInfo rootCoordInfo = new CoordInfo()
            {
                Center = center,
                Range = new Vector2(terrainSize, terrainSize)
            };

            m_Root = new QuadTreeRootNode(rootCoordInfo);

            BuildTree(m_Root);
        }

        private void BuildTree(QuadTreeNode root)
        {
            for(QuadTreeAreaType childNodeType = QuadTreeAreaType.LeftUp; childNodeType <= QuadTreeAreaType.LeftBottom; childNodeType++)
            {
                QuadTreeNode childNode = root.InsertChild(childNodeType);
                if (childNode != null)
                {
                    BuildTree(childNode);
                }
            }
        }

        public void HandleObject(Vector3 pos, bool isAdd)
        {
            if (m_Root != null)
            {
                m_Root.HandleObject(pos, isAdd);
            }
        }
    }
}