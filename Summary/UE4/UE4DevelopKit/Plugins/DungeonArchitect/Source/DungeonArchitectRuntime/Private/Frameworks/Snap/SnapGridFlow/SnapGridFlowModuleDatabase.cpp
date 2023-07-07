//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleDatabase.h"

#include "Core/Utils/MathUtils.h"
#include "Core/Utils/Stats.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphConstraints.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/Lib/FlowAbstractGraphPathUtils.h"

//////////////////////////// Snap Grid Flow Module Assembly ////////////////////////////

const int32 FSGFModuleAssemblySide::INDEX_VALID_UNKNOWN = -2;

void FSGFModuleAssemblySide::Init(int32 InWidth, int32 InHeight) {
    Width = InWidth;
    Height = InHeight;
    ConnectionIndices.SetNum(Width * Height);
}

FSGFModuleAssemblySide FSGFModuleAssemblySide::Rotate90CW() const {
    FSGFModuleAssemblySide RotatedSide;
    RotatedSide.Init(Height, Width);
    for (int32 x = 0; x < Width; x++) {
        for (int32 y = 0; y < Height; y++) {
            const FSGFModuleAssemblySideCell& Cell = Get(x, y);
            const int32 xx = Height - 1 - y;
            const int32 yy = x;
            RotatedSide.Set(xx, yy, Cell);
        }
    }
    
    return MoveTemp(RotatedSide);
}

void FSGFModuleAssembly::Initialize(const FIntVector& InNumChunks) {
    NumChunks = InNumChunks;
    Front.Init(NumChunks.X, NumChunks.Z);
    Left.Init(NumChunks.Y, NumChunks.Z);
    Back.Init(NumChunks.X, NumChunks.Z);
    Right.Init(NumChunks.Y, NumChunks.Z);

    Top.Init(NumChunks.X, NumChunks.Y);
    Down.Init(NumChunks.X, NumChunks.Y);
}

bool FSGFModuleAssembly::CanFit(const FSGFModuleAssembly& AssemblyToFit, TArray<FSGFModuleAssemblySideCell>& OutDoorIndices) const {
    if (NumChunks != AssemblyToFit.NumChunks) return false;
    const FSGFModuleAssemblySide* HostSides[] = { &Front, &Left, &Back, &Right, &Top, &Down };
    const FSGFModuleAssemblySide* TargetSides[] = { &AssemblyToFit.Front, &AssemblyToFit.Left, &AssemblyToFit.Back, &AssemblyToFit.Right, &AssemblyToFit.Top, &AssemblyToFit.Down };
    OutDoorIndices.Reset();
    for (int s = 0; s < 6; s++) {
        const FSGFModuleAssemblySide& HostSide = *HostSides[s];
        const FSGFModuleAssemblySide& TargetSide = *TargetSides[s];
        check(HostSide.Width == TargetSide.Width && HostSide.Height == TargetSide.Height);
        const int32 NumEntries = HostSide.ConnectionIndices.Num();
        for (int i = 0; i < NumEntries; i++) {
            const bool bHostContainsConnection = HostSide.ConnectionIndices[i].HasConnection();
            const bool bTargetRequiresConnection = TargetSide.ConnectionIndices[i].HasConnection();
            if (bTargetRequiresConnection) {
                if (bHostContainsConnection) {
                    FSGFModuleAssemblySideCell DoorCell;
                    DoorCell.ConnectionIdx = HostSide.ConnectionIndices[i].ConnectionIdx;
                    DoorCell.NodeId = TargetSide.ConnectionIndices[i].NodeId;
                    DoorCell.LinkedNodeId = TargetSide.ConnectionIndices[i].LinkedNodeId;
                    OutDoorIndices.Add(DoorCell);
                }
                else {
                    return false;
                }
            }
        }
    }
    return true;
}

void FSGFModuleAssemblyBuilder::Build(const FVector& InChunkSize,
                                                const FSnapGridFlowModuleDatabaseItem& InModuleInfo,
                                                FSGFModuleAssembly& OutAssembly) {
    const FIntVector& NumChunks = InModuleInfo.NumChunks;
    OutAssembly = FSGFModuleAssembly();
    OutAssembly.Initialize(NumChunks);

    enum class EAssemblySide : uint8 { Unknown, Front, Left, Back, Right, Down, Top };
    
    const FVector BaseOffset = -InModuleInfo.ModuleBounds.Min;
    for (int32 ConnectionIdx = 0; ConnectionIdx < InModuleInfo.Connections.Num(); ConnectionIdx++) {
        const FSnapGridFlowModuleDatabaseConnectionInfo& ConnectionInfo = InModuleInfo.Connections[ConnectionIdx];
        FVector ConnectionLocation = ConnectionInfo.Transform.GetLocation() + BaseOffset;
        FVector LocalChunkPosition = ConnectionLocation / InChunkSize;
        const FVector& P = LocalChunkPosition;

        struct FAssemblyAttachmentDistance {
            EAssemblySide Side = EAssemblySide::Unknown;
            float Distance = 0;
        };

        /*
        if (FMath::IsNearlyEqual(P.Y, 0.0f)) BestSide = EAssemblySide::Front;
        else if (FMath::IsNearlyEqual(P.X, NumChunks.X)) BestSide = EAssemblySide::Left;
        else if (FMath::IsNearlyEqual(P.Y, NumChunks.Y)) BestSide = EAssemblySide::Back;
        else if (FMath::IsNearlyEqual(P.X, 0)) BestSide = EAssemblySide::Right;
        else if (FMath::IsNearlyEqual(P.Z, 0)) BestSide = EAssemblySide::Down;
        else if (FMath::IsNearlyEqual(P.Z, NumChunks.Z)) BestSide = EAssemblySide::Top;
        */

        TArray<FAssemblyAttachmentDistance> AttachmentDistances;
        AttachmentDistances.Add({ EAssemblySide::Front, FMath::Abs(P.Y) });
        AttachmentDistances.Add({ EAssemblySide::Left, FMath::Abs(P.X - NumChunks.X) });
        AttachmentDistances.Add({ EAssemblySide::Back, FMath::Abs(P.Y - NumChunks.Y) });
        AttachmentDistances.Add({ EAssemblySide::Right, FMath::Abs(P.X) });
        AttachmentDistances.Add({ EAssemblySide::Down, FMath::Abs(P.Z) });
        AttachmentDistances.Add({ EAssemblySide::Top, FMath::Abs(P.Z - NumChunks.Z) });
        
        EAssemblySide BestSide = EAssemblySide::Unknown;
        float BestDistance = MAX_flt;
        for (const FAssemblyAttachmentDistance& AttachDistInfo : AttachmentDistances) {
            if (AttachDistInfo.Distance < BestDistance) {
                BestDistance = AttachDistInfo.Distance;
                BestSide = AttachDistInfo.Side;
            }
        }
        
        if (BestSide == EAssemblySide::Front) {
            // Front
            const int32 CX = FMath::FloorToInt(P.X);
            const int32 CY = FMath::FloorToInt(P.Z);
            OutAssembly.Front.Set(CX, CY, ConnectionIdx);
            //UE_LOG(LogSnapGridFlowLib, Log, TEXT("FRONT: Location (%.2f, %.2f)"), P.X, P.Y);
        }
        else if (BestSide == EAssemblySide::Left) {
            // Left
            const int32 CX = FMath::FloorToInt(P.Y);
            const int32 CY = FMath::FloorToInt(P.Z);
            OutAssembly.Left.Set(CX, CY, ConnectionIdx);
            //UE_LOG(LogSnapGridFlowLib, Log, TEXT("LEFT: Location (%.2f, %.2f)"), P.X, P.Y);
        }
        else if (BestSide == EAssemblySide::Back) {
            // Back
            const int32 CX = NumChunks.X - 1 - FMath::FloorToInt(P.X);
            const int32 CY = FMath::FloorToInt(P.Z);
            OutAssembly.Back.Set(CX, CY, ConnectionIdx);
            //UE_LOG(LogSnapGridFlowLib, Log, TEXT("BACK: Location (%.2f, %.2f)"), P.X, P.Y);
        }
        else if (BestSide == EAssemblySide::Right) {
            // Right
            const int32 CX = NumChunks.Y - 1 - FMath::FloorToInt(P.Y);
            const int32 CY = FMath::FloorToInt(P.Z);
            OutAssembly.Right.Set(CX, CY, ConnectionIdx);
            //UE_LOG(LogSnapGridFlowLib, Log, TEXT("RIGHT: Location (%.2f, %.2f)"), P.X, P.Y);
        }
        else if (BestSide == EAssemblySide::Down) {
            // Down
            const int32 CX = FMath::FloorToInt(P.X);
            const int32 CY = FMath::FloorToInt(P.Y);
            OutAssembly.Down.Set(CX, CY, ConnectionIdx);
            //UE_LOG(LogSnapGridFlowLib, Log, TEXT("DOWN: Location (%.2f, %.2f)"), P.X, P.Y);
        }
        else if (BestSide == EAssemblySide::Top) {
            // Top
            const int32 CX = FMath::FloorToInt(P.X);
            const int32 CY = FMath::FloorToInt(P.Y);
            OutAssembly.Top.Set(CX, CY, ConnectionIdx);
            //UE_LOG(LogSnapGridFlowLib, Log, TEXT("TOP: Location (%.2f, %.2f)"), P.X, P.Y);
        }
    }
}

void FSGFModuleAssemblyBuilder::Build(const FFlowAbstractGraphQuery& InGraphQuery, const FFlowAGPathNodeGroup& Group,
        const TArray<FFAGConstraintsLink>& IncomingNodes, FSGFModuleAssembly& OutAssembly) {
    
    FVector MinCoordF, MaxCoordF;
    for (int i = 0; i < Group.GroupEdgeNodes.Num(); i++) {
        FGuid EdgeNodeId = Group.GroupEdgeNodes[i];
        UFlowAbstractNode* EdgeNode = InGraphQuery.GetNode(EdgeNodeId);
        if (!EdgeNode) {
            EdgeNode = InGraphQuery.GetSubNode(EdgeNodeId);
        }
        check(EdgeNode);
        
        if (i == 0) {
            MinCoordF = MaxCoordF = EdgeNode->Coord; 
        }
        else {
            MinCoordF = MinCoordF.ComponentMin(EdgeNode->Coord);
            MaxCoordF = MaxCoordF.ComponentMax(EdgeNode->Coord);
        }
    }

    const FIntVector MinCoord = FMathUtils::ToIntVector(MinCoordF, true);
    const FIntVector MaxCoord = FMathUtils::ToIntVector(MaxCoordF, true);

    const FIntVector NumChunks = MaxCoord - MinCoord + FIntVector(1, 1, 1);
    OutAssembly.Initialize(NumChunks);
    for (const FFAGConstraintsLink& Link : IncomingNodes) {
        if (!Link.IncomingNode) continue;
        if (!Link.Node) continue;
        
        const FIntVector C = FMathUtils::ToIntVector(Link.Node->Coord, true) - MinCoord;
        const FIntVector IC = FMathUtils::ToIntVector(Link.IncomingNode->Coord, true) - MinCoord;
        
        FSGFModuleAssemblySideCell Cell;
        Cell.ConnectionIdx = FSGFModuleAssemblySide::INDEX_VALID_UNKNOWN;
        Cell.NodeId = Link.Node ? Link.Node->NodeId : FGuid();
        Cell.LinkedNodeId = Link.IncomingNode ? Link.IncomingNode->NodeId : FGuid();
        
        if (C.Y > IC.Y) {
            // Front
            OutAssembly.Front.Set(C.X, C.Z, Cell);
        }
        else if (IC.X > C.X) {
            // Left
            OutAssembly.Left.Set(C.Y, C.Z, Cell);
        }
        else if (IC.Y > C.Y) {
            // Back
            OutAssembly.Back.Set(NumChunks.X - 1 - C.X, C.Z, Cell);
        }
        else if (C.X > IC.X) {
            // Right
            OutAssembly.Right.Set(NumChunks.Y - 1 - C.Y, C.Z, Cell);
        }
        else if (C.Z > IC.Z) {
            // Down
            OutAssembly.Down.Set(C.X, C.Y, Cell);
        }
        else if (IC.Z > C.Z) {
            // Top
            OutAssembly.Top.Set(C.X, C.Y, Cell);
        }
    }
}

void FSGFModuleAssemblyBuilder::Rotate90CW(const FSGFModuleAssembly& InAssembly, FSGFModuleAssembly& OutAssembly) {
    OutAssembly = FSGFModuleAssembly();
    OutAssembly.NumChunks = FIntVector(InAssembly.NumChunks.Y, InAssembly.NumChunks.X, InAssembly.NumChunks.Z);
    OutAssembly.Left = InAssembly.Front;
    OutAssembly.Back = InAssembly.Left;
    OutAssembly.Right = InAssembly.Back;
    OutAssembly.Front = InAssembly.Right;
    OutAssembly.Top = InAssembly.Top.Rotate90CW();
    OutAssembly.Down = InAssembly.Down.Rotate90CW();
}

