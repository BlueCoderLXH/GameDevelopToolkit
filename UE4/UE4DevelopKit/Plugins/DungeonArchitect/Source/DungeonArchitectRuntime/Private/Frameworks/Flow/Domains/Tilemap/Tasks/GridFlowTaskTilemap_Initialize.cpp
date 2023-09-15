//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_Initialize.h"

#include "Core/Utils/Attributes.h"
#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

#include "Containers/Queue.h"

DEFINE_LOG_CATEGORY_STATIC(LogCreateTilemapTask, Log, All);

namespace {
    struct FDoorInfo {
        FGuid AbstractLinkId;
        FIntPoint CellCoord;
        bool bHorizontalDoorEdge;
    };

    FIntPoint ScaleIntPoint(const FIntPoint& A, const FIntPoint& B) {
        return FIntPoint(A.X * B.X, A.Y * B.Y);
    }

    struct FNodeTilemapBounds {
        static FNodeTilemapBounds Build(const FGridFlowTilemapNodeInfo& TileNode, int32 TilemapWidth,
                                        int32 TilemapHeight, bool bWallsAsTiles) {
            FNodeTilemapBounds b;

            b.x0 = FMath::FloorToInt(TileNode.x0);
            b.y0 = FMath::FloorToInt(TileNode.y0);
            b.x1 = FMath::FloorToInt(TileNode.x1);
            b.y1 = FMath::FloorToInt(TileNode.y1);
            b.mx = FMath::FloorToInt(TileNode.midX);
            b.my = FMath::FloorToInt(TileNode.midY);

            if (bWallsAsTiles) {
                b.x0 = FMath::Clamp(b.x0, 0, TilemapWidth - 1);
                b.x1 = FMath::Clamp(b.x1, 0, TilemapWidth - 1);
                b.y0 = FMath::Clamp(b.y0, 0, TilemapHeight - 1);
                b.y1 = FMath::Clamp(b.y1, 0, TilemapHeight - 1);
                b.mx = FMath::Clamp(b.mx, 0, TilemapWidth - 1);
                b.my = FMath::Clamp(b.my, 0, TilemapHeight - 1);
            }

            return b;
        }

        int32 x0 = 0;
        int32 x1 = 0;
        int32 y0 = 0;
        int32 y1 = 0;
        int32 mx = 0;
        int32 my = 0;
    };
}


void UGridFlowTaskTilemap_Initialize::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
    if (Input.IncomingNodeOutputs.Num() == 0) {
        Output.ErrorMessage = "Missing Input";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }
    if (Input.IncomingNodeOutputs.Num() > 1) {
        Output.ErrorMessage = "Only one input allowed";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    Output.State = Input.IncomingNodeOutputs[0].State->Clone();

    UGridFlowAbstractGraph* Graph = Output.State->GetState<UGridFlowAbstractGraph>(UFlowAbstractGraphBase::StateTypeID);
    if (!Graph) {
        Output.ErrorMessage = "Invalid Input";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    UGridFlowTilemap* Tilemap = BuildTilemap(Graph, *Input.Random);
    Output.State->SetStateObject(UGridFlowTilemap::StateTypeID, Tilemap);
    Output.ExecutionResult = EFlowTaskExecutionResult::Success;
}

UGridFlowTilemap* UGridFlowTaskTilemap_Initialize::BuildTilemap(UGridFlowAbstractGraph* Graph,
                                                                const FRandomStream& Random) {
    const FFlowAbstractGraphQuery GraphQuery(Graph);
    LayoutGridSize = Graph->GridSize;
    const FIntPoint TilemapSize = ScaleIntPoint(LayoutGridSize, TilemapSizePerNode);
    UGridFlowTilemap* Tilemap = NewObject<UGridFlowTilemap>();
    Tilemap->Initialize(TilemapSize.X, TilemapSize.Y);

    FGridFlowTilemapNodes TileNodes;
    TileNodes.InitializeArray2D(LayoutGridSize.X, LayoutGridSize.Y);
    for (int ny = 0; ny < LayoutGridSize.Y; ny++) {
        for (int nx = 0; nx < LayoutGridSize.X; nx++) {
            int32 x0 = nx * TilemapSizePerNode.X;
            int32 y0 = ny * TilemapSizePerNode.Y;
            int32 x1 = x0 + TilemapSizePerNode.X;
            int32 y1 = y0 + TilemapSizePerNode.Y;
            TileNodes[FIntPoint(nx, ny)] = FGridFlowTilemapNodeInfo(x0, y0, x1, y1);
        }
    }

    for (const UFlowAbstractNode* Node : Graph->GraphNodes) {
        FIntVector Coord = FMathUtils::ToIntVector(Node->Coord, true);
        TileNodes[Coord].AbstractNodeId = Node->NodeId;
    }

    PerturbRoomSizes(TileNodes, Tilemap, GraphQuery, Random);
    FixCorridorSizes(TileNodes, GraphQuery);

    InitRasterization(TileNodes, Tilemap, GraphQuery);
    RasterizeRoomCorridors(TileNodes, Tilemap, GraphQuery);
    RasterizeBaseCaveBlocks(TileNodes, Tilemap, GraphQuery);

    GenerateMainPath(TileNodes, Tilemap, GraphQuery);
    BuildCaves(TileNodes, Tilemap, GraphQuery, Random);
    BuildDoors(TileNodes, Tilemap, GraphQuery);

    Tilemap = ApplyCropAndPadding(Tilemap);

    CalculateDistanceFromMainPath(TileNodes, Tilemap, GraphQuery, {EGridFlowAbstractNodeRoomType::Cave});
    CalculateDistanceFromMainPath(TileNodes, Tilemap, GraphQuery, {
                                      EGridFlowAbstractNodeRoomType::Room,
                                      EGridFlowAbstractNodeRoomType::Corridor,
                                  });
    CalculateDistanceFromMainPathOnEmptyArea(Tilemap);

    return Tilemap;
}

void UGridFlowTaskTilemap_Initialize::PerturbRoomSizes(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                                       const FFlowAbstractGraphQuery& GraphQuery,
                                                       const FRandomStream& Random) {
    UGridFlowAbstractGraph* Graph = GraphQuery.GetGraph<UGridFlowAbstractGraph>();
    CorridorLaneWidth = FMath::Max(1, CorridorLaneWidth);

    // Perturb horizontally
    {
        int32 MaxPerturbAmount = FMath::FloorToInt((TilemapSizePerNode.X - 2 - CorridorLaneWidth) * 0.5f);
        MaxPerturbAmount = FMath::Max(0, MaxPerturbAmount);

        int32 PerturbX = FMath::Min(PerturbAmount.X, MaxPerturbAmount);
        PerturbX = FMath::Max(0, PerturbX);

        for (int ny = 0; ny < LayoutGridSize.Y; ny++) {
            for (int nx = -1; nx < LayoutGridSize.X; nx++) {
                FGridFlowTilemapNodeInfo* NodeA = TileNodes.GetSafe(nx, ny);
                FGridFlowTilemapNodeInfo* NodeB = TileNodes.GetSafe(nx + 1, ny);

                bool bConnected = false;
                if (NodeA && NodeB) {
                    UFlowAbstractLink* Link = Graph->GetLink(NodeA->AbstractNodeId, NodeB->AbstractNodeId, true);
                    bConnected = (Link && Link->Type != EFlowAbstractLinkType::Unconnected);
                }
                UFlowAbstractNode* AbstractNodeA = NodeA ? GraphQuery.GetNode(NodeA->AbstractNodeId) : nullptr;
                UFlowAbstractNode* AbstractNodeB = NodeB ? GraphQuery.GetNode(NodeB->AbstractNodeId) : nullptr;

                const bool bDisablePerturbA = AbstractNodeA ? AbstractNodeA->FindOrAddDomainData<UFANodeTilemapDomainData>()->bDisablePerturb : false;
                const bool bDisablePerturbB = AbstractNodeB ? AbstractNodeB->FindOrAddDomainData<UFANodeTilemapDomainData>()->bDisablePerturb : false;
                
                if (bConnected) {
                    const bool bCanPerturb = (AbstractNodeA && AbstractNodeB
                        && !bDisablePerturbA
                        && !bDisablePerturbB);
                    if (bCanPerturb) {
                        const float Perturb = Random.FRandRange(-PerturbX, PerturbX);
                        if (NodeA) NodeA->x1 += Perturb;
                        if (NodeB) NodeB->x0 += Perturb;
                    }
                }
                else {
                    if (NodeA && AbstractNodeA && !bDisablePerturbA) {
                        const float PerturbA = PerturbX * Random.FRand();
                        NodeA->x1 -= PerturbA;
                    }
                    if (NodeB && AbstractNodeB && !bDisablePerturbB) {
                        const float PerturbB = PerturbX * Random.FRand();
                        NodeB->x0 += PerturbB;
                    }
                }
            }
        }
    }

    // Perturb vertically
    {
        int32 MaxPerturbAmount = FMath::FloorToInt((TilemapSizePerNode.Y - 2 - CorridorLaneWidth) * 0.5f);
        MaxPerturbAmount = FMath::Max(0, MaxPerturbAmount);

        int32 PerturbY = FMath::Min(PerturbAmount.Y, MaxPerturbAmount);
        PerturbY = FMath::Max(0, PerturbY);

        for (int nx = 0; nx < LayoutGridSize.X; nx++) {
            for (int ny = -1; ny < LayoutGridSize.Y; ny++) {
                FGridFlowTilemapNodeInfo* NodeA = TileNodes.GetSafe(nx, ny);
                FGridFlowTilemapNodeInfo* NodeB = TileNodes.GetSafe(nx, ny + 1);

                bool bConnected = false;
                if (NodeA && NodeB) {
                    UFlowAbstractLink* Link = Graph->GetLink(NodeA->AbstractNodeId, NodeB->AbstractNodeId, true);
                    bConnected = (Link && Link->Type != EFlowAbstractLinkType::Unconnected);
                }

                UFlowAbstractNode* AbstractNodeA = NodeA ? GraphQuery.GetNode(NodeA->AbstractNodeId) : nullptr;
                UFlowAbstractNode* AbstractNodeB = NodeB ? GraphQuery.GetNode(NodeB->AbstractNodeId) : nullptr;

                const bool bDisablePerturbA = AbstractNodeA ? AbstractNodeA->FindOrAddDomainData<UFANodeTilemapDomainData>()->bDisablePerturb : false;
                const bool bDisablePerturbB = AbstractNodeB ? AbstractNodeB->FindOrAddDomainData<UFANodeTilemapDomainData>()->bDisablePerturb : false;
                
                if (bConnected) {
                    const bool bCanPerturb = AbstractNodeA && AbstractNodeB
                        && !bDisablePerturbA
                        && !bDisablePerturbB;

                    if (bCanPerturb) {
                        bool bCanMoveDown = (NodeA->x0 >= NodeB->x0 && NodeA->x1 <= NodeB->x1);
                        bool bCanMoveUp = (NodeB->x0 >= NodeA->x0 && NodeB->x1 <= NodeA->x1);

                        if (!bCanMoveUp && !bCanMoveDown) continue;

                        if (bCanMoveUp && bCanMoveDown) {
                            // Move randomly on either one direction
                            if (Random.FRand() < 0.5f) {
                                bCanMoveUp = false;
                            }
                            else {
                                bCanMoveDown = false;
                            }
                        }

                        const float PerturbDirection = (bCanMoveUp ? -1 : 1);
                        const float Perturb = Random.FRand() * PerturbY * PerturbDirection;
                        NodeA->y1 += Perturb;
                        NodeB->y0 += Perturb;
                    }
                }
                else {
                    if (NodeA && AbstractNodeA && !bDisablePerturbA) {
                        const float PerturbA = PerturbY * Random.FRand();
                        NodeA->y1 -= PerturbA;
                    }

                    if (NodeB && AbstractNodeB && !bDisablePerturbB) {
                        const float PerturbB = PerturbY * Random.FRand();
                        NodeB->y0 += PerturbB;
                    }
                }
            }
        }
    }
}

void UGridFlowTaskTilemap_Initialize::FixCorridorSizes(FGridFlowTilemapNodes& TileNodes,
                                                       const FFlowAbstractGraphQuery& GraphQuery) const {
    for (FGridFlowTilemapNodeInfo& TileNode : TileNodes.GetCells()) {
        UFlowAbstractNode* Node = GraphQuery.GetNode(TileNode.AbstractNodeId);
        if (Node && Node->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType == EGridFlowAbstractNodeRoomType::Corridor) {
            UGridFlowAbstractGraph* Graph = GraphQuery.GetGraph<UGridFlowAbstractGraph>();
            TArray<FGuid> IncomingNodes = Graph->GetIncomingNodes(Node->NodeId);
            TArray<FGuid> OutgoingNodes = Graph->GetOutgoingNodes(Node->NodeId);
            if (IncomingNodes.Num() == 0 || OutgoingNodes.Num() == 0) continue;

            UFlowAbstractNode* IncomingNode = GraphQuery.GetNode(IncomingNodes[0]);
            UFlowAbstractNode* OutgoingNode = GraphQuery.GetNode(OutgoingNodes[0]);
            if (!IncomingNode || !OutgoingNode) continue;

            const FIntVector InCoord = FMathUtils::ToIntVector(IncomingNode->Coord, true);
            const FIntVector OutCoord = FMathUtils::ToIntVector(OutgoingNode->Coord, true);
            const bool bVertical = (InCoord.X == OutCoord.X);
            if (bVertical) {
                TileNode.x0 = FMath::Max(TileNode.x0, TileNode.midX - CorridorLaneWidth);
                TileNode.x1 = FMath::Min(TileNode.x1, TileNode.midX + CorridorLaneWidth);
            }
            else {
                TileNode.y0 = FMath::Max(TileNode.y0, TileNode.midY - CorridorLaneWidth);
                TileNode.y1 = FMath::Min(TileNode.y1, TileNode.midY + CorridorLaneWidth);
            }
        }
    }
}

void UGridFlowTaskTilemap_Initialize::InitRasterization(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                                        const FFlowAbstractGraphQuery& GraphQuery) const {
    for (FGridFlowTilemapNodeInfo& TileNode : TileNodes.GetCells()) {
        UFlowAbstractNode* TileNodePtr = GraphQuery.GetNode(TileNode.AbstractNodeId);
        if (!TileNodePtr) continue;

        const bool bWallsAsTiles = WallGenerationMethod == EGridFlowTilemapWallGenerationMethod::WallAsTiles;
        const FNodeTilemapBounds Bounds = FNodeTilemapBounds::Build(TileNode, Tilemap->GetWidth(), Tilemap->GetHeight(), bWallsAsTiles);
        for (int y = Bounds.y0; y <= Bounds.y1; y++) {
            for (int x = Bounds.x0; x <= Bounds.x1; x++) {
                FGridFlowTilemapCell* CellPtr = Tilemap->GetSafe(x, y);
                if (!CellPtr) continue;
                CellPtr->ChunkCoord = TileNodePtr->Coord;
            }
        }
    }
}

void UGridFlowTaskTilemap_Initialize::RasterizeRoomCorridors(FGridFlowTilemapNodes& TileNodes,
                                                             UGridFlowTilemap* Tilemap,
                                                             const FFlowAbstractGraphQuery& GraphQuery) const {
    const bool bWallsAsTiles = WallGenerationMethod == EGridFlowTilemapWallGenerationMethod::WallAsTiles;
    for (FGridFlowTilemapNodeInfo& TileNodeInfo : TileNodes.GetCells()) {
        UFlowAbstractNode* TileNode = GraphQuery.GetNode(TileNodeInfo.AbstractNodeId);
        if (!TileNode || !TileNode->bActive) continue;

        const FNodeTilemapBounds Bounds = FNodeTilemapBounds::Build(TileNodeInfo, Tilemap->GetWidth(), Tilemap->GetHeight(), bWallsAsTiles);

        if (TileNode->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType == EGridFlowAbstractNodeRoomType::Cave) {
            // Render the caves in another pass
            continue;
        }

        for (int y = Bounds.y0; y <= Bounds.y1; y++) {
            for (int x = Bounds.x0; x <= Bounds.x1; x++) {
                FGridFlowTilemapCell* CellPtr = Tilemap->GetSafe(x, y);
                if (!CellPtr) continue;

                FGridFlowTilemapCell& Cell = *CellPtr;
                Cell.bLayoutCell = true;
                Cell.ChunkCoord = TileNode->Coord;
                if (bWallsAsTiles) {
                    Cell.CellType = (x == Bounds.x0 || x == Bounds.x1 || y == Bounds.y0 || y == Bounds.y1)
                                        ? EGridFlowTilemapCellType::Wall
                                        : EGridFlowTilemapCellType::Floor;
                }
                else {
                    if (x < Bounds.x1 && y < Bounds.y1) {
                        Cell.CellType = EGridFlowTilemapCellType::Floor;
                    }
                }

                FLinearColor NodeColor = TileNode->Color;
                Cell.CustomColor = FColorUtils::BrightenColor(NodeColor, RoomColorSaturation, RoomColorBrightness);

                if (Cell.CellType == EGridFlowTilemapCellType::Floor) {
                    Cell.bUseCustomColor = true;
                }
            }
        }
        
        // Rasterize the edges
        if (!bWallsAsTiles)
        {
            for (int y = Bounds.y0; y < Bounds.y1; y++)
            {
                {
                    FGridFlowTilemapEdge& Edge = Tilemap->GetEdgeV(Bounds.x0, y);
                    Edge.EdgeType = EGridFlowTilemapEdgeType::Wall;
                    Edge.MarkerAngle = 90;
                    
                    FGridFlowTilemapCellWallInfo WallInfo;
                    WallInfo.OwningTiles.Add(FIntPoint(Bounds.x0, y));
                    Tilemap->SetWallMetadata(Edge.EdgeCoord, WallInfo);
                }
                
                {
                    FGridFlowTilemapEdge& Edge = Tilemap->GetEdgeV(Bounds.x1, y);
                    Edge.EdgeType = EGridFlowTilemapEdgeType::Wall;
                    Edge.MarkerAngle = -90;

                    FGridFlowTilemapCellWallInfo WallInfo;
                    WallInfo.OwningTiles.Add(FIntPoint(Bounds.x1 - 1, y));
                    Tilemap->SetWallMetadata(Edge.EdgeCoord, WallInfo);
                }
            }
            for (int x = Bounds.x0; x < Bounds.x1; x++)
            {
                {
                    FGridFlowTilemapEdge& Edge = Tilemap->GetEdgeH(x, Bounds.y0);
                    Edge.EdgeType = EGridFlowTilemapEdgeType::Wall;
                    Edge.MarkerAngle = 180;

                    FGridFlowTilemapCellWallInfo WallInfo;
                    WallInfo.OwningTiles.Add(FIntPoint(x, Bounds.y0));
                    Tilemap->SetWallMetadata(Edge.EdgeCoord, WallInfo);
                }
                
                {
                    FGridFlowTilemapEdge& Edge = Tilemap->GetEdgeH(x, Bounds.y1);
                    Edge.EdgeType = EGridFlowTilemapEdgeType::Wall;
                    Edge.MarkerAngle = 0;
                    
                    FGridFlowTilemapCellWallInfo WallInfo;
                    WallInfo.OwningTiles.Add(FIntPoint(x, Bounds.y1 - 1));
                    Tilemap->SetWallMetadata(Edge.EdgeCoord, WallInfo);
                }
            }
        }
    }
}

void UGridFlowTaskTilemap_Initialize::GenerateMainPath(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                                       const FFlowAbstractGraphQuery& GraphQuery) {
    UGridFlowAbstractGraph* Graph = GraphQuery.GetGraph<UGridFlowAbstractGraph>();
    for (const UFlowAbstractLink* Link : Graph->GraphLinks) {
        UFlowAbstractNode* NodeA = GraphQuery.GetNode(Link->Source);
        UFlowAbstractNode* NodeB = GraphQuery.GetNode(Link->Destination);
        if (!NodeA || !NodeB) continue;

        const FIntPoint TileCenterA = NodeCoordToTileCoord(NodeA->Coord);
        const FIntPoint TileCenterB = NodeCoordToTileCoord(NodeB->Coord);
        if (TileCenterA.X == TileCenterB.X) {
            int32 x = TileCenterA.X;
            int32 y0 = FMath::Min(TileCenterA.Y, TileCenterB.Y);
            int32 y1 = FMath::Max(TileCenterA.Y, TileCenterB.Y);
            for (int y = y0; y <= y1; y++) {
                FGridFlowTilemapCell* CellPtr = Tilemap->GetSafe(x, y);
                if (!CellPtr) continue;

                FGridFlowTilemapCell& Cell = *CellPtr;
                Cell.bMainPath = true;
                Cell.DistanceFromMainPath = 0;
            }
        }
        else if (TileCenterA.Y == TileCenterB.Y) {
            int32 y = TileCenterA.Y;
            int x0 = FMath::Min(TileCenterA.X, TileCenterB.X);
            int x1 = FMath::Max(TileCenterA.X, TileCenterB.X);
            for (int x = x0; x <= x1; x++) {
                FGridFlowTilemapCell* CellPtr = Tilemap->GetSafe(x, y);
                if (!CellPtr) continue;

                FGridFlowTilemapCell& Cell = *CellPtr;
                Cell.bMainPath = true;
                Cell.DistanceFromMainPath = 0;
            }
        }
        else {
            UE_LOG(LogCreateTilemapTask, Error, TEXT("Invalid Input"));
        }
    }

    if (Graph->GraphLinks.Num() == 0 && Graph->GraphNodes.Num() == 1) {
        UFlowAbstractNode* Node = Cast<UFlowAbstractNode>(Graph->GraphNodes[0]);
        const FIntPoint TileCoord = NodeCoordToTileCoord(Node->Coord);
        FGridFlowTilemapCell& Cell = Tilemap->Get(TileCoord.X, TileCoord.Y);
        Cell.bMainPath = true;
        Cell.DistanceFromMainPath = 0;
    }
}

void UGridFlowTaskTilemap_Initialize::BuildCaves(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                                 const FFlowAbstractGraphQuery& GraphQuery,
                                                 const FRandomStream& Random) {
    CalculateDistanceFromMainPath(TileNodes, Tilemap, GraphQuery, {EGridFlowAbstractNodeRoomType::Cave});

    FGFCaveCellBuildTiles CaveMap;
    GenerateCaveMap(CaveMap, TileNodes, Tilemap, GraphQuery);
    BuildCaveStep_BuildRocks(CaveMap, Tilemap, Random);
    BuildCaveStep_SimulateGrowth(CaveMap, Tilemap, Random);
    BuildCaveStep_Cleanup(CaveMap, TileNodes, Tilemap, GraphQuery);
    BuildCaveStep_UpdateEdges(CaveMap, Tilemap);
}

void UGridFlowTaskTilemap_Initialize::GenerateCaveMap(FGFCaveCellBuildTiles& CaveMap, FGridFlowTilemapNodes& TileNodes,
                                                      UGridFlowTilemap* Tilemap,
                                                      const FFlowAbstractGraphQuery& GraphQuery) {
    CaveMap.InitializeArray2D(Tilemap->GetWidth(), Tilemap->GetHeight());

    for (const FGridFlowTilemapCell& Cell : Tilemap->GetCells()) {
        FGFCaveCellBuildTile& CaveTile = CaveMap[Cell.TileCoord];
        CaveTile.TileCoord = Cell.TileCoord;

        FGridFlowTilemapNodeInfo& TileNode = TileNodes[FMathUtils::ToIntVector(Cell.ChunkCoord, true)];
        UFlowAbstractNode* TileNodePtr = GraphQuery.GetNode(TileNode.AbstractNodeId);
        if (TileNodePtr) {
            CaveTile.bValid = (TileNodePtr->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType == EGridFlowAbstractNodeRoomType::Cave
                        && TileNodePtr->bActive && Cell.bLayoutCell);
        }
    }
}

void UGridFlowTaskTilemap_Initialize::BuildCaveStep_BuildRocks(FGFCaveCellBuildTiles& CaveMap,
                                                               UGridFlowTilemap* Tilemap, const FRandomStream& Random) const {
    for (FGFCaveCellBuildTile& CaveCell : CaveMap.GetCells()) {
        FGridFlowTilemapCell* TileCell = Tilemap->GetSafe(CaveCell.TileCoord.X, CaveCell.TileCoord.Y);
        if (!TileCell) continue;
        if (CaveCell.bValid) {
            if (CaveThickness > 0) {
                const float RockProbability = FMath::Exp(-TileCell->DistanceFromMainPath / CaveThickness);
                CaveCell.bRockTile = Random.FRand() < RockProbability;
            }
            else {
                CaveCell.bRockTile = (TileCell->DistanceFromMainPath == 0);
            }
        }
    }
}

void UGridFlowTaskTilemap_Initialize::BuildCaveStep_SimulateGrowth(FGFCaveCellBuildTiles& CaveMap,
                                                                   UGridFlowTilemap* Tilemap,
                                                                   const FRandomStream& Random) const {
    const int32 Width = CaveMap.GetWidth();
    const int32 Height = CaveMap.GetHeight();

    for (int i = 0; i < CaveAutomataIterations; i++) {
        FGFCaveCellBuildTiles OldMap = CaveMap;

        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                int nrocks = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx < 0 || ny < 0 || nx >= Width || ny >= Height) continue;
                        if (OldMap[FIntPoint(nx, ny)].bRockTile) {
                            nrocks++;
                        }
                    }
                }

                if (nrocks >= CaveAutomataNeighbors) {
                    CaveMap[FIntPoint(x, y)].bRockTile = true;
                }
            }
        }

    }

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            if (CaveMap[FIntPoint(x, y)].bValid && !CaveMap[FIntPoint(x, y)].bRockTile) {
                Tilemap->Get(x, y).CellType = EGridFlowTilemapCellType::Empty;
                Tilemap->Get(x, y).bUseCustomColor = false;
                CaveMap[FIntPoint(x, y)].bValid = false;
            }
        }
    }
}

void UGridFlowTaskTilemap_Initialize::BuildCaveStep_Cleanup(FGFCaveCellBuildTiles& CaveMap,
                                                            FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                                            const FFlowAbstractGraphQuery& GraphQuery) const {
    int32 Width = Tilemap->GetWidth();
    int32 Height = Tilemap->GetHeight();
    TDAArray2D<bool> TraversibleCaveTiles;
    TraversibleCaveTiles.InitializeArray2D(Width, Height);

    static const int childOffsets[] = {
        -1, 0,
        1, 0,
        0, -1,
        0, 1
    };

    for (FGridFlowTilemapNodeInfo& TileNode : TileNodes.GetCells()) {
        UFlowAbstractNode* TileNodePtr = GraphQuery.GetNode(TileNode.AbstractNodeId);
        if (!TileNodePtr) continue;
        if (TileNodePtr->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType != EGridFlowAbstractNodeRoomType::Cave) {
            // Only process the caves
            continue;
        }

        if (!TileNodePtr->bActive) {
            // Do not process inactive nodes
            continue;
        }

        FIntPoint tileCenter = NodeCoordToTileCoord(TileNodePtr->Coord);

        if (TraversibleCaveTiles[tileCenter]) {
            // Already processed from another adjacent node
            continue;
        }

        // Flood fill from the center of this node
        TQueue<FIntPoint> Queue;
        Queue.Enqueue(tileCenter);
        FIntPoint Front;
        while (Queue.Dequeue(Front)) {
            if (TraversibleCaveTiles[Front]) {
                // Already processed
                continue;
            }

            TraversibleCaveTiles[Front] = true;

            // Traverse the children
            for (int i = 0; i < 4; i++) {
                FIntPoint ChildCoord(
                    Front.X + childOffsets[i * 2 + 0],
                    Front.Y + childOffsets[i * 2 + 1]);

                if (ChildCoord.X >= 0 && ChildCoord.Y >= 0 && ChildCoord.X < Width && ChildCoord.Y < Height) {
                    if (CaveMap[ChildCoord].bValid) {
                        bool bVisited = TraversibleCaveTiles[ChildCoord];
                        if (!bVisited) {
                            Queue.Enqueue(ChildCoord);
                        }
                    }
                }
            }
        }
    }

    // Assign the valid traversable paths 
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            FGridFlowTilemapCell& Cell = Tilemap->Get(x, y);
            FIntVector NodeCoord = FMathUtils::ToIntVector(Cell.ChunkCoord, true);
            FGridFlowTilemapNodeInfo& TileNode = TileNodes[NodeCoord];
            UFlowAbstractNode* TileNodePtr = GraphQuery.GetNode(TileNode.AbstractNodeId);
            if (!TileNodePtr) continue;

            if (TileNodePtr->bActive && TileNodePtr->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType == EGridFlowAbstractNodeRoomType::Cave) {
                const bool bValid = TraversibleCaveTiles[FIntPoint(x, y)];
                CaveMap[FIntPoint(x, y)].bValid = bValid;
                if (!bValid || !CaveMap[FIntPoint(x, y)].bRockTile) {
                    Tilemap->Get(x, y).CellType = EGridFlowTilemapCellType::Empty;
                    Tilemap->Get(x, y).bUseCustomColor = false;
                    Tilemap->Get(x, y).bLayoutCell = false;
                }
                else {
                    Tilemap->Get(x, y).bLayoutCell = true;
                }
            }
            else {
                check(CaveMap[FIntPoint(x, y)].bValid == false);
            }
        }
    }


    if (bDebugLayoutTiles) {
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                FGridFlowTilemapCell& Cell = Tilemap->Get(x, y);
                FIntPoint Coord(x, y);
                if (CaveMap[Coord].bValid) {
                    Cell.CustomColor = FLinearColor::Green;
                    Cell.bUseCustomColor = true;
                }
                if (TraversibleCaveTiles[Coord]) {
                    //Cell.CustomColor = FLinearColor::Red;
                    //Cell.bUseCustomColor = true;
                }
            }
        }
    }
}


namespace {
    bool CanCreateEdgeToAdjacentCaveTile(UGridFlowTilemap* Tilemap, FGFCaveCellBuildTiles& CaveMap, int x, int y)
    {
        if (x < 0 || x >= Tilemap->GetWidth() || y < 0 || y >= Tilemap->GetHeight()) return true;   // this is an empty tile

        if (Tilemap->Get(x, y).CellType == EGridFlowTilemapCellType::Empty
            || Tilemap->Get(x, y).CellType == EGridFlowTilemapCellType::Custom)
        {
            return true;
        }

        const bool bCaveTile = CaveMap[FIntPoint(x, y)].bValid && CaveMap[FIntPoint(x, y)].bRockTile;
        if (bCaveTile)
        {
            // no need for an edge between two cave tiles
            return false;
        }

        // This tile is not empty and is not a cave tile
        return false;
    }

}

void UGridFlowTaskTilemap_Initialize::BuildCaveStep_UpdateEdges(FGFCaveCellBuildTiles& CaveMap, UGridFlowTilemap* Tilemap) {
    const int32 Width = Tilemap->GetWidth();
    const int32 Height = Tilemap->GetHeight();

    for (int y = 0; y < Height; y++)
    {
        for (int x = 0; x < Width; x++)
        {
            const bool bCaveTile = x < Width && y < Height && CaveMap[FIntPoint(x, y)].bValid && CaveMap[FIntPoint(x, y)].bRockTile;
            if (!bCaveTile) continue;

            const bool bCreateEdgeLeft = CanCreateEdgeToAdjacentCaveTile(Tilemap, CaveMap, x - 1, y);
            const bool bCreateEdgeRight = CanCreateEdgeToAdjacentCaveTile(Tilemap, CaveMap, x + 1, y);

            const bool bCreateEdgeBottom = CanCreateEdgeToAdjacentCaveTile(Tilemap, CaveMap, x, y - 1);
            const bool bCreateEdgeTop = CanCreateEdgeToAdjacentCaveTile(Tilemap, CaveMap, x, y + 1);

            if (bCreateEdgeLeft)
            {
                FGridFlowTilemapEdge& EdgeV = Tilemap->GetEdgeV(x, y);
                if (EdgeV.EdgeType == EGridFlowTilemapEdgeType::Empty)
                {
                    EdgeV.EdgeType = EGridFlowTilemapEdgeType::Fence;
                    EdgeV.MarkerAngle = -90;
                }
            }

            if (bCreateEdgeRight)
            {
                FGridFlowTilemapEdge& EdgeV = Tilemap->GetEdgeV(x + 1, y);
                if (EdgeV.EdgeType == EGridFlowTilemapEdgeType::Empty)
                {
                    EdgeV.EdgeType = EGridFlowTilemapEdgeType::Fence;
                    EdgeV.MarkerAngle = 90;
                }
            }

            if (bCreateEdgeBottom)
            {
                FGridFlowTilemapEdge& EdgeH = Tilemap->GetEdgeH(x, y);
                if (EdgeH.EdgeType == EGridFlowTilemapEdgeType::Empty)
                {
                    EdgeH.EdgeType = EGridFlowTilemapEdgeType::Fence;
                    EdgeH.MarkerAngle = 0;
                }
            }

            if (bCreateEdgeTop)
            {
                FGridFlowTilemapEdge& EdgeH = Tilemap->GetEdgeH(x, y + 1);
                if (EdgeH.EdgeType == EGridFlowTilemapEdgeType::Empty)
                {
                    EdgeH.EdgeType = EGridFlowTilemapEdgeType::Fence;
                    EdgeH.MarkerAngle = 180;
                }
            }
        }
    }
}

void UGridFlowTaskTilemap_Initialize::BuildDoors(FGridFlowTilemapNodes& TileNodes, UGridFlowTilemap* Tilemap,
                                                 const FFlowAbstractGraphQuery& GraphQuery) const {
    UGridFlowAbstractGraph* Graph = GraphQuery.GetGraph<UGridFlowAbstractGraph>();
    const bool bWallsAsTiles = (WallGenerationMethod == EGridFlowTilemapWallGenerationMethod::WallAsTiles);
    // Build the doors
    TArray<FDoorInfo> DoorList;
    for (FGridFlowTilemapNodeInfo& TileNode : TileNodes.GetCells()) {
        UFlowAbstractNode* TileNodePtr = GraphQuery.GetNode(TileNode.AbstractNodeId);
        if (!TileNodePtr || !TileNodePtr->bActive) continue;

        FNodeTilemapBounds b = FNodeTilemapBounds::Build(TileNode, Tilemap->GetWidth(), Tilemap->GetHeight(), bWallsAsTiles);

        UFlowAbstractNode* Node = TileNodePtr;
        const FVector NodeCoord = TileNodePtr->Coord;
        for (FGuid LinkId : Graph->GetOutgoingLinks(TileNodePtr->NodeId)) {
            UFlowAbstractLink* Link = Graph->FindLink(LinkId);
            if (!Link || Link->Type == EFlowAbstractLinkType::Unconnected) continue;
            FGridFlowTilemapCell* DoorCell = nullptr;

            UFlowAbstractNode* OtherNode = GraphQuery.GetNode(Link->Destination);
            if (Node->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType == EGridFlowAbstractNodeRoomType::Cave
                    && OtherNode->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType == EGridFlowAbstractNodeRoomType::Cave) {
                // We don't need a door between two cave nodes
                continue;
            }

            const FVector OtherCoord = OtherNode->Coord;
            float Angle = 0;
            bool bHorizontalEdge = true;
            if (NodeCoord.X == OtherCoord.X) {
                // Vertical link
                int32 y = (NodeCoord.Y < OtherCoord.Y) ? b.y1 : b.y0;
                DoorCell = Tilemap->GetSafe(b.mx, y);
                Angle = 90;
                bHorizontalEdge = true;
            }
            else if (NodeCoord.Y == OtherCoord.Y) {
                // Horizontal link
                int32 x = (NodeCoord.X < OtherCoord.X) ? b.x1 : b.x0;
                DoorCell = Tilemap->GetSafe(x, b.my);
                bHorizontalEdge = false;
            }

            if (DoorCell) {
                UFlowAbstractNode* SourceNode = GraphQuery.GetNode(Link->Source);
                UFlowAbstractNode* DestNode = GraphQuery.GetNode(Link->Destination);

                FGridFlowTilemapCellDoorInfo DoorMeta;
                DoorMeta.bOneWay = Link->Type == EFlowAbstractLinkType::OneWay;
                DoorMeta.ChunkA = FMathUtils::ToIntVector(SourceNode->Coord, true);
                DoorMeta.ChunkB = FMathUtils::ToIntVector(DestNode->Coord, true);

                // Find the correct angle
                {
                    if (DoorMeta.ChunkA.Y > DoorMeta.ChunkB.Y) {
                        // Pointing Up
                        Angle = 0;
                    }
                    else if (DoorMeta.ChunkA.X < DoorMeta.ChunkB.X) {
                        // Pointing Right
                        Angle = 90;
                    }
                    else if (DoorMeta.ChunkA.Y < DoorMeta.ChunkB.Y) {
                        // Pointing Down
                        Angle = 180;
                    }
                    else if (DoorMeta.ChunkA.X > DoorMeta.ChunkB.X) {
                        // Pointing Left
                        Angle = 270;
                    }
                }

                DoorMeta.Angle = Angle;
                int32 NumLockedItems = 0;
                for (const UFlowGraphItem* Item : Link->LinkItems) {
                    if (Item->ItemType == EFlowGraphItemType::Lock) {
                        NumLockedItems++;
                    }
                }
                DoorMeta.bLocked = NumLockedItems > 0;

                if (bWallsAsTiles) {
                    DoorCell->CellType = EGridFlowTilemapCellType::Door;
                    Tilemap->SetDoorMetadata(FGridFlowTilemapCoord(DoorCell->TileCoord), DoorMeta);
                }
                else {
                    FGridFlowTilemapEdge& DoorEdge = bHorizontalEdge
                        ? Tilemap->GetEdgeH(DoorCell->TileCoord.X, DoorCell->TileCoord.Y)
                        : Tilemap->GetEdgeV(DoorCell->TileCoord.X, DoorCell->TileCoord.Y);
                    DoorEdge.EdgeType = EGridFlowTilemapEdgeType::Door;
                    DoorEdge.MarkerAngle = Angle;
                    Tilemap->SetDoorMetadata(FGridFlowTilemapCoord(DoorCell->TileCoord, bHorizontalEdge), DoorMeta);
                }

                FDoorInfo& DoorInfo = DoorList.AddDefaulted_GetRef();
                DoorInfo.AbstractLinkId = Link->LinkId;
                DoorInfo.CellCoord = DoorCell->TileCoord;
                DoorInfo.bHorizontalDoorEdge = bHorizontalEdge;
            }
        }
    }

    // Add door items
    for (const FDoorInfo& Door : DoorList) {
        UFlowAbstractLink* Link = Graph->FindLink(Door.AbstractLinkId);
        TArray<UFlowGraphItem*>& Items = Link->LinkItems;
        UFlowGraphItem* DoorItem = Items.Num() > 0 ? Items[0] : nullptr;
        if (!DoorItem) continue;

        // TODO: Add metadata in the model with source/destination, one-way etc

        const FIntPoint CellCoord = Door.CellCoord;
        if (bWallsAsTiles) {
            FGridFlowTilemapCell& Cell = Tilemap->Get(CellCoord.X, CellCoord.Y);
            Cell.ItemId = DoorItem->ItemId;
            Cell.bHasItem = true;
        }
        else {
            FGridFlowTilemapEdge* DoorEdge = Door.bHorizontalDoorEdge
                ? Tilemap->GetEdgeHSafe(CellCoord.X, CellCoord.Y)
                : Tilemap->GetEdgeVSafe(CellCoord.X, CellCoord.Y);

            if (DoorEdge) {
                DoorEdge->ItemId = DoorItem->ItemId;
                DoorEdge->bHasItem = true;
            }
        }
    }
}

UGridFlowTilemap* UGridFlowTaskTilemap_Initialize::ApplyCropAndPadding(UGridFlowTilemap* OldTilemap) const {
    if (OldTilemap->GetWidth() == 0 || OldTilemap->GetHeight() == 0) {
        return OldTilemap;
    }

    int x0 = 0;
    int x1 = 0;
    int y0 = 0;
    int y1 = 0;

    if (bCropTilemapToLayout) {
        bool bFoundFirstCell = false;
        for (const FGridFlowTilemapCell& Cell : OldTilemap->GetCells()) {
            bool bLayoutTile = Cell.CellType == EGridFlowTilemapCellType::Floor
                || Cell.CellType == EGridFlowTilemapCellType::Wall
                || Cell.CellType == EGridFlowTilemapCellType::Door;

            if (bLayoutTile) {
                int32 x = Cell.TileCoord.X;
                int32 y = Cell.TileCoord.Y;
                if (!bFoundFirstCell) {
                    bFoundFirstCell = true;
                    x0 = x1 = x;
                    y0 = y1 = y;
                }
                else {
                    x0 = FMath::Min(x0, x);
                    x1 = FMath::Max(x1, x);
                    y0 = FMath::Min(y0, y);
                    y1 = FMath::Max(y1, y);
                }
            }
        }
    }
    else {
        // No cropping required
        x0 = 0;
        x1 = OldTilemap->GetWidth() - 1;
        y0 = 0;
        y1 = OldTilemap->GetHeight() - 1;
    }

    int32 p = LayoutPadding;
    int32 LayoutWidth = x1 - x0 + 1;
    int32 LayoutHeight = y1 - y0 + 1;
    UGridFlowTilemap* Tilemap = NewObject<UGridFlowTilemap>(OldTilemap->GetOuter());
    Tilemap->Initialize(LayoutWidth + p * 2, LayoutHeight + p * 2);

    for (int y = 0; y < LayoutHeight; y++) {
        for (int x = 0; x < LayoutWidth; x++) {
            int32 ix = x + p;
            int32 iy = y + p;
            FIntPoint Coord(ix, iy);

            // Copy over the cell
            {
                FGridFlowTilemapCell Cell = OldTilemap->Get(x + x0, y + y0);
                Cell.TileCoord = Coord;
                Tilemap->Set(ix, iy, Cell);
            }

        }
    }

    for (int y = 0; y <= LayoutHeight; y++) {
        for (int x = 0; x <= LayoutWidth; x++) {
            int32 ix = x + p;
            int32 iy = y + p;
            FIntPoint Coord(ix, iy);
            
            // Copy over the horizontal edge
            {
                FGridFlowTilemapEdge Edge = OldTilemap->GetEdgeH(x + x0, y + y0);
                Edge.EdgeCoord = FGridFlowTilemapCoord(Coord, true);
                Tilemap->SetEdgeH(ix, iy, Edge);
            }

            // Copy over the vertical edge
            {
                FGridFlowTilemapEdge Edge = OldTilemap->GetEdgeV(x + x0, y + y0);
                Edge.EdgeCoord = FGridFlowTilemapCoord(Coord, false);
                Tilemap->SetEdgeV(ix, iy, Edge);
            }
        }
    }
    
#define TRANSLATE(Coord) ((Coord) - FIntPoint(x0, y0) + FIntPoint(p, p))

    // Update the metadata
    {
        // Fix the wall metadata coords
        for (auto& Entry : OldTilemap->GetWallMetaMap()) {
            // Fix the wall info
            FGridFlowTilemapCellWallInfo WallInfo = Entry.Value;
            for (int i = 0; i < WallInfo.OwningTiles.Num(); i++) {
                WallInfo.OwningTiles[i] = TRANSLATE(WallInfo.OwningTiles[i]);
            }

            const FGridFlowTilemapCoord& OldTileCoord = Entry.Key;
            FIntPoint NewCoordPoint = TRANSLATE(OldTileCoord.Coord);
            const FGridFlowTilemapCoord NewCoord(NewCoordPoint, OldTileCoord.bHorizontalEdge);
            Tilemap->SetWallMetadata(NewCoord, WallInfo);
        }

        // Fix the door metadata coords
        for (auto& Entry : OldTilemap->GetDoorMetaMap()) {
            FGridFlowTilemapCellDoorInfo DoorInfo = Entry.Value;
            //DoorInfo.ChunkA = TRANSLATE(DoorInfo.ChunkA);
            //DoorInfo.ChunkB = TRANSLATE(DoorInfo.NodeB);

            const FGridFlowTilemapCoord& OldTileCoord = Entry.Key;
            FIntPoint NewCoordPoint = TRANSLATE(OldTileCoord.Coord);
            FGridFlowTilemapCoord NewCoord;
            NewCoord.Coord = NewCoordPoint;
            NewCoord.bIsEdgeCoord = OldTileCoord.bIsEdgeCoord;
            NewCoord.bHorizontalEdge = OldTileCoord.bHorizontalEdge;
            Tilemap->SetDoorMetadata(NewCoord, DoorInfo);
        }
    }

#undef TRANSLATE

    return Tilemap;
}

void UGridFlowTaskTilemap_Initialize::CalculateDistanceFromMainPathOnEmptyArea(UGridFlowTilemap* Tilemap) const {
    const int32 Width = Tilemap->GetWidth();
    const int32 Height = Tilemap->GetHeight();
    TQueue<FGridFlowTilemapCell*> Queue;

    static const int ChildOffsets[] =
    {
        -1, 0,
        1, 0,
        0, -1,
        0, 1
    };

    for (FGridFlowTilemapCell& Cell : Tilemap->GetCells()) {
        if (Cell.CellType != EGridFlowTilemapCellType::Empty) {
            continue;
        }

        bool bValidStartNode = false;

        for (int i = 0; i < 4; i++) {
            int nx = Cell.TileCoord.X + ChildOffsets[i * 2 + 0];
            int ny = Cell.TileCoord.Y + ChildOffsets[i * 2 + 1];
            if (nx >= 0 && nx < Width && ny >= 0 && ny < Height) {
                FGridFlowTilemapCell& ncell = Tilemap->Get(nx, ny);
                if (ncell.CellType != EGridFlowTilemapCellType::Empty) {
                    bValidStartNode = true;
                    Cell.DistanceFromMainPath = FMath::Min(Cell.DistanceFromMainPath, ncell.DistanceFromMainPath);
                }
            }
        }

        if (bValidStartNode) {
            Queue.Enqueue(&Cell);
        }
    }


    FGridFlowTilemapCell* Cell = nullptr;
    while (Queue.Dequeue(Cell)) {
        int32 ndist = Cell->DistanceFromMainPath + 1;

        for (int i = 0; i < 4; i++) {
            int nx = Cell->TileCoord.X + ChildOffsets[i * 2 + 0];
            int ny = Cell->TileCoord.Y + ChildOffsets[i * 2 + 1];
            if (nx >= 0 && nx < Width && ny >= 0 && ny < Height) {
                FGridFlowTilemapCell& ncell = Tilemap->Get(nx, ny);

                if (ncell.CellType == EGridFlowTilemapCellType::Empty) {
                    if (ndist < ncell.DistanceFromMainPath) {
                        ncell.DistanceFromMainPath = ndist;
                        Queue.Enqueue(&ncell);

                    }
                }
            }
        }
    }
}

void UGridFlowTaskTilemap_Initialize::RasterizeBaseCaveBlocks(FGridFlowTilemapNodes& TileNodes,
                                                              UGridFlowTilemap* Tilemap,
                                                              const FFlowAbstractGraphQuery& GraphQuery) const {
    UGridFlowAbstractGraph* Graph = GraphQuery.GetGraph<UGridFlowAbstractGraph>();
    for (FGridFlowTilemapNodeInfo& TileNode : TileNodes.GetCells()) {
        UFlowAbstractNode* TileNodePtr = GraphQuery.GetNode(TileNode.AbstractNodeId);
        if (!TileNodePtr || !TileNodePtr->bActive) continue;

        const bool bWallsAsTiles = (WallGenerationMethod == EGridFlowTilemapWallGenerationMethod::WallAsTiles);
        FNodeTilemapBounds b = FNodeTilemapBounds::Build(TileNode, Tilemap->GetWidth(), Tilemap->GetHeight(), bWallsAsTiles);
        if (TileNodePtr->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType != EGridFlowAbstractNodeRoomType::Cave) {
            // Only build the caves in this pass
            continue;
        }

        UFlowAbstractNode* CaveNode = TileNodePtr;
        const bool bBlockLeft = ShouldBlockCaveBoundary(Graph, CaveNode, -1, 0);
        const bool bBlockRight = ShouldBlockCaveBoundary(Graph, CaveNode, 1, 0);
        const bool bBlockTop = ShouldBlockCaveBoundary(Graph, CaveNode, 0, -1);
        const bool bBlockBottom = ShouldBlockCaveBoundary(Graph, CaveNode, 0, 1);
        for (int y = b.y0; y <= b.y1; y++) {
            for (int x = b.x0; x <= b.x1; x++) {
                FGridFlowTilemapCell* CellPtr = Tilemap->GetSafe(x, y);
                if (!CellPtr) continue;
                FGridFlowTilemapCell& Cell = *CellPtr;

                if (Cell.CellType == EGridFlowTilemapCellType::Empty) {
                    bool bMakeFloor = true;
                    if (bWallsAsTiles) {
                        if (bBlockLeft && x == b.x0) bMakeFloor = false;
                        if (bBlockRight && x == b.x1) bMakeFloor = false;
                        if (bBlockTop && y == b.y0) bMakeFloor = false;
                        if (bBlockBottom && y == b.y1) bMakeFloor = false;
                    }
                    else {
                        if (bBlockRight && x == b.x1) bMakeFloor = false;
                        if (bBlockBottom && y == b.y1) bMakeFloor = false;
                    }

                    if (bMakeFloor) {
                        Cell.ChunkCoord = TileNodePtr->Coord;
                        Cell.CellType = EGridFlowTilemapCellType::Floor;
                        Cell.bUseCustomColor = true;
                        FLinearColor nodeColor = TileNodePtr->Color;
                        Cell.CustomColor = FColorUtils::BrightenColor(nodeColor, RoomColorSaturation,
                                                                      RoomColorBrightness);
                    }

                    Cell.bLayoutCell = bMakeFloor;
                }
            }
        }
    }
}

bool UGridFlowTaskTilemap_Initialize::ShouldBlockCaveBoundary(UGridFlowAbstractGraph* Graph, UFlowAbstractNode* CaveNode, int dx, int dy) const {
    const FIntVector Coord = FMathUtils::ToIntVector(CaveNode->Coord, true);
    const FIntVector OtherCoord = Coord + FIntVector(dx, dy, 0);
    UFlowAbstractNode* OtherNode = nullptr;
    for (UFlowAbstractNode* BaseNode : Graph->GraphNodes) {
        UFlowAbstractNode* Node = Cast<UFlowAbstractNode>(BaseNode);
        if (Node && FMathUtils::ToIntVector(Node->Coord, true) == OtherCoord) {
            OtherNode = Node;
            break;
        }
    }

    if (!OtherNode || !OtherNode->bActive) {
        // a node in this location doesn't exist
        return false;
    }

    // Check if we have a link between these nodes. If we don't, then block it
    UFlowAbstractLink* Link = Graph->GetLink(CaveNode->NodeId, OtherNode->NodeId);
    if (!Link) {
        // No link exists. we should block this
        return true;
    }

    // We have a link to the other node.   block only if they it is a non-cave nodes
    return OtherNode->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType != EGridFlowAbstractNodeRoomType::Cave;
}

FIntPoint UGridFlowTaskTilemap_Initialize::NodeCoordToTileCoord(const FVector& NodeCoord) const {
    FIntPoint TileCoord = ScaleIntPoint(FIntPoint(NodeCoord.X, NodeCoord.Y), TilemapSizePerNode);
    TileCoord += TilemapSizePerNode / 2;
    return TileCoord;
}

void UGridFlowTaskTilemap_Initialize::CalculateDistanceFromMainPath(FGridFlowTilemapNodes& TileNodes,
                                                                    UGridFlowTilemap* Tilemap,
                                                                    const FFlowAbstractGraphQuery& GraphQuery,
                                                                    const TArray<EGridFlowAbstractNodeRoomType>
                                                                    AllowedRoomTypes) const {
    const int32 Width = Tilemap->GetWidth();
    const int32 Height = Tilemap->GetHeight();
    TQueue<FGridFlowTilemapCell*> Queue;

    for (FGridFlowTilemapCell& Cell : Tilemap->GetCells()) {
        FGridFlowTilemapNodeInfo& TileNode = TileNodes[FMathUtils::ToIntVector(Cell.ChunkCoord, true)];
        UFlowAbstractNode* TileNodePtr = GraphQuery.GetNode(TileNode.AbstractNodeId);
        if (!TileNodePtr) continue;

        EGridFlowAbstractNodeRoomType RoomType = TileNodePtr->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType;
        if (!AllowedRoomTypes.Contains(RoomType)) {
            continue;
        }

        if (Cell.bMainPath) {
            Queue.Enqueue(&Cell);
        }
    }

    static int ChildOffsets[] = {
        -1, 0,
        1, 0,
        0, -1,
        0, 1
    };

    FGridFlowTilemapCell* Tile = nullptr;
    while (Queue.Dequeue(Tile)) {
        // Traverse the children
        const int32 ChildDistance = Tile->DistanceFromMainPath + 1;
        for (int i = 0; i < 4; i++) {
            int nx = Tile->TileCoord.X + ChildOffsets[i * 2 + 0];
            int ny = Tile->TileCoord.Y + ChildOffsets[i * 2 + 1];
            if (nx >= 0 && nx < Width && ny >= 0 && ny < Height) {
                FGridFlowTilemapCell& NCell = Tilemap->Get(nx, ny);
                FGridFlowTilemapNodeInfo& NTileNode = TileNodes[FMathUtils::ToIntVector(NCell.ChunkCoord, true)];
                UFlowAbstractNode* NTileNodePtr = GraphQuery.GetNode(NTileNode.AbstractNodeId);
                EGridFlowAbstractNodeRoomType NRoomType = NTileNodePtr->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType;
                if (!AllowedRoomTypes.Contains(NRoomType)) {
                    continue;
                }

                if (ChildDistance < NCell.DistanceFromMainPath) {
                    NCell.DistanceFromMainPath = ChildDistance;
                    Queue.Enqueue(&NCell);
                }
            }
        }
    }
}


bool UGridFlowTaskTilemap_Initialize::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    FLOWTASKATTR_GET_SIZE(TilemapSizePerNode);
    FLOWTASKATTR_GET_SIZE(PerturbAmount);
    FLOWTASKATTR_GET_INT(CorridorLaneWidth);
    FLOWTASKATTR_GET_INT(LayoutPadding);
    FLOWTASKATTR_GET_BOOL(bCropTilemapToLayout);
    FLOWTASKATTR_GET_INT(CaveAutomataNeighbors);
    FLOWTASKATTR_GET_INT(CaveAutomataIterations);
    FLOWTASKATTR_GET_FLOAT(CaveThickness);

    return false;
}

bool UGridFlowTaskTilemap_Initialize::SetParameter(const FString& InParameterName, const FDAAttribute& InValue) {
    FLOWTASKATTR_SET_SIZEI(TilemapSizePerNode);
    FLOWTASKATTR_SET_SIZEI(PerturbAmount);
    FLOWTASKATTR_SET_INT(CorridorLaneWidth);
    FLOWTASKATTR_SET_INT(LayoutPadding);
    FLOWTASKATTR_SET_BOOL(bCropTilemapToLayout);
    FLOWTASKATTR_SET_INT(CaveAutomataNeighbors);
    FLOWTASKATTR_SET_INT(CaveAutomataIterations);
    FLOWTASKATTR_SET_FLOAT(CaveThickness);

    return false;
}

bool UGridFlowTaskTilemap_Initialize::SetParameterSerialized(const FString& InParameterName,
                                                             const FString& InSerializedText) {
    FLOWTASKATTR_SET_PARSE_SIZEI(TilemapSizePerNode);
    FLOWTASKATTR_SET_PARSE_SIZEI(PerturbAmount);
    FLOWTASKATTR_SET_PARSE_INT(CorridorLaneWidth);
    FLOWTASKATTR_SET_PARSE_INT(LayoutPadding);
    FLOWTASKATTR_SET_PARSE_BOOL(bCropTilemapToLayout);
    FLOWTASKATTR_SET_PARSE_INT(CaveAutomataNeighbors);
    FLOWTASKATTR_SET_PARSE_INT(CaveAutomataIterations);
    FLOWTASKATTR_SET_PARSE_FLOAT(CaveThickness);

    return false;
}

