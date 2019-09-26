using UnityEngine;

using Algorithm.QuadTree;

public class TerrainInfo : MonoBehaviour
{
    public static TerrainInfo Ins { get; private set; }

    [SerializeField] Color m_GridLineColor = Color.green;

    [SerializeField] Transform m_LeftUp;
    [SerializeField] Transform m_RightUp;
    [SerializeField] Transform m_RightBottom;
    [SerializeField] Transform m_LeftBottom;

    public Transform LeftUp => m_LeftUp;
    public Transform RightUp => m_RightUp;
    public Transform RightBottom => m_RightBottom;
    public Transform LeftBottom => m_LeftBottom;

    QuadTree m_QuadTree;

    private void Awake()
    {
        Ins = this;

        m_QuadTree = new QuadTree(Vector3.zero, QuadTreeConfig.TerrainSize);

        int terrainSize = QuadTreeConfig.TerrainSize;
        int gridSize = QuadTreeConfig.GridSize;

        for (int x = -terrainSize / 2 + gridSize / 2; x < terrainSize / 2; x += gridSize)
        {
            //Debug.DrawLine(new Vector3(x, 0, -terrainSize / 2), new Vector3(x, 0, terrainSize / 2), m_GridLineColor);
            for (int z = -terrainSize / 2 + gridSize / 2; z < terrainSize / 2; z += gridSize)
            {
                //Debug.DrawLine(new Vector3(-terrainSize / 2, 0, z), new Vector3(terrainSize / 2, 0, z), m_GridLineColor);
                Vector3 pos = new Vector3(x, 0, z);
                m_QuadTree.HandleObject(pos, true);
            }
        }
    }

    private void Update()
    {
        int terrainSize = QuadTreeConfig.TerrainSize;
        int gridSize = QuadTreeConfig.GridSize;

        for (int x = -terrainSize / 2; x <= terrainSize / 2; x += gridSize)
        {
            Debug.DrawLine(new Vector3(x, 0, -terrainSize / 2), new Vector3(x, 0, terrainSize / 2), m_GridLineColor);
        }

        for (int z = -terrainSize / 2; z <= terrainSize / 2; z += gridSize)
        {
            Debug.DrawLine(new Vector3(-terrainSize / 2, 0, z), new Vector3(terrainSize / 2, 0, z), m_GridLineColor);
        }
    }
}