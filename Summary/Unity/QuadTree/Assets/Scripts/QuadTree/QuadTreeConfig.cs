using System;

namespace Algorithm.QuadTree
{
    /// <summary>
    /// 四叉树配置信息
    /// </summary>
    public static class QuadTreeConfig
    {
        // 地形大小
        public const int TerrainSize = 256;

        public const int TerrainHeight = 1;

        // 网格大小(最小划分单位)
        public static int GridSize
        {
            get { return TerrainSize / (int)Math.Pow(2, MaxLayer); }
        }

        // 子节点数
        public const int ChildCount = 4;

        // 根节点layer
        public const int RootLayer = 0;

        // 叶节点layer
        public const int MaxLayer = 4;
    }
}