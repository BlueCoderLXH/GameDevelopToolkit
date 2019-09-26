using System.Collections.Generic;
using UnityEngine;

namespace Algorithm.QuadTree
{
    /// <summary>
    /// 四叉树叶节点
    /// </summary>
    public class QuadTreeLeafNode : QuadTreeNode
    {
        List<GameObject> m_GameObjects;

        public QuadTreeLeafNode(QuadTreeAreaType areaType, CoordInfo coordRange)
            : base(areaType, coordRange, QuadTreeConfig.MaxLayer, true)
        {
            m_GameObjects = new List<GameObject>();
        }

        protected override void OnHandleObject(Vector3 pos, bool isAdd)
        {
            //if (isAdd)
            //{
            //    m_GameObjects.Add(gameObject);
            //}
            //else
            //{
            //    m_GameObjects.Remove(gameObject);
            //}

            Vector3 newPos = pos;
            newPos.y = QuadTreeConfig.TerrainHeight;

            GameObject gObj = null;
            switch (m_AreaType)
            {
                case QuadTreeAreaType.LeftUp:
                    gObj = Object.Instantiate(Resources.Load("Prefabs/NorthWest")) as GameObject;
                    gObj.transform.SetParent(TerrainInfo.Ins.LeftUp);
                    break;

                case QuadTreeAreaType.RightUp:
                    gObj = Object.Instantiate(Resources.Load("Prefabs/NorthEast")) as GameObject;
                    gObj.transform.SetParent(TerrainInfo.Ins.RightUp);
                    break;

                case QuadTreeAreaType.RightBottom:
                    gObj = Object.Instantiate(Resources.Load("Prefabs/SouthEast")) as GameObject;
                    gObj.transform.SetParent(TerrainInfo.Ins.RightBottom);
                    break;

                case QuadTreeAreaType.LeftBottom:
                    gObj = Object.Instantiate(Resources.Load("Prefabs/SouthWest")) as GameObject;
                    gObj.transform.SetParent(TerrainInfo.Ins.LeftBottom);
                    break;
            }

            gObj.transform.position = newPos;
            gObj.transform.localScale = new Vector3(1.6f, 1.0f, 1.6f);
        }
    }
}