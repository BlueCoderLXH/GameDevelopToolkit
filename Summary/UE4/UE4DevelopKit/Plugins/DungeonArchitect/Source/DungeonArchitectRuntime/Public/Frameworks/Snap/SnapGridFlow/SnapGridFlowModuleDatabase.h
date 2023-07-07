//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionConstants.h"
#include "Frameworks/ThemeEngine/Markers/PlaceableMarker.h"
#include "SnapGridFlowModuleDatabase.generated.h"

struct FFlowAGPathNodeGroup;
struct FFAGConstraintsLink;
class USnapGridFlowModuleBoundsAsset;
template<typename TNode> class TFlowAbstractGraphQuery; 
typedef TFlowAbstractGraphQuery<class UFlowAbstractNode> FFlowAbstractGraphQuery;

//////////////////////////// Snap Grid Flow Module Assembly ////////////////////////////

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSGFModuleAssemblySideCell {
    GENERATED_BODY()
    FSGFModuleAssemblySideCell() {}
    FSGFModuleAssemblySideCell(int32 InConnectionIdx)
        : ConnectionIdx(InConnectionIdx)
    {
    }

    UPROPERTY()
    int32 ConnectionIdx = INDEX_NONE;

    UPROPERTY()
    FGuid NodeId;

    UPROPERTY()
    FGuid LinkedNodeId;
    
    UPROPERTY()
    FGuid LinkId;

    FORCEINLINE bool HasConnection() const { return ConnectionIdx != INDEX_NONE; }
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSGFModuleAssemblySide {
    GENERATED_BODY()

    UPROPERTY()
    int32 Width = 0;
    
    UPROPERTY()
    int32 Height = 0;
    
    UPROPERTY()
    TArray<FSGFModuleAssemblySideCell> ConnectionIndices;

    void Init(int32 InWidth, int32 InHeight);

    FORCEINLINE FSGFModuleAssemblySideCell& Get(int32 X, int32 Y) {
        check(X >= 0 && X < Width && Y >= 0 && Y < Height);
        return ConnectionIndices[Y * Width + X];
    }

    FORCEINLINE const FSGFModuleAssemblySideCell& Get(int32 X, int32 Y) const {
        check(X >= 0 && X < Width && Y >= 0 && Y < Height);
        return ConnectionIndices[Y * Width + X];
    }

    FORCEINLINE void Set(int32 X, int32 Y, const FSGFModuleAssemblySideCell& InCell) {
        check(X >= 0 && X < Width && Y >= 0 && Y < Height);
        ConnectionIndices[Y * Width + X] = InCell;
    }
    
    FSGFModuleAssemblySide Rotate90CW() const;
    
    static const int32 INDEX_VALID_UNKNOWN;
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSGFModuleAssembly {
    GENERATED_BODY()
    
    UPROPERTY()
    FIntVector NumChunks;
    
    UPROPERTY()
    FSGFModuleAssemblySide Front;
    
    UPROPERTY()
    FSGFModuleAssemblySide Left;
    
    UPROPERTY()
    FSGFModuleAssemblySide Back;
    
    UPROPERTY()
    FSGFModuleAssemblySide Right;
    
    UPROPERTY()
    FSGFModuleAssemblySide Top;
    
    UPROPERTY()
    FSGFModuleAssemblySide Down;

    void Initialize(const FIntVector& InNumChunks);
    bool CanFit(const FSGFModuleAssembly& AssemblyToFit, TArray<FSGFModuleAssemblySideCell>& OutDoorIndices) const;
};

class DUNGEONARCHITECTRUNTIME_API FSGFModuleAssemblyBuilder {
public:
    static void Build(const FVector& InChunkSize, const struct FSnapGridFlowModuleDatabaseItem& InModuleInfo, FSGFModuleAssembly& OutAssembly);
    static void Build(const FFlowAbstractGraphQuery& InGraphQuery, const FFlowAGPathNodeGroup& Group, const TArray<FFAGConstraintsLink>& IncomingNodes, FSGFModuleAssembly& OutAssembly);

    static void Rotate90CW(const FSGFModuleAssembly& InAssembly, FSGFModuleAssembly& OutAssembly);
};

//////////////////////////// Snap Grid Flow Module Database ////////////////////////////
USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSnapGridFlowModuleDatabaseConnectionInfo {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, Category = Module)
    FGuid ConnectionId;

    UPROPERTY(VisibleAnywhere, Category = Module)
    FTransform Transform;

    UPROPERTY(VisibleAnywhere, Category = Module)
    class USnapConnectionInfo* ConnectionInfo;

    UPROPERTY(VisibleAnywhere, Category = Module)
    ESnapConnectionConstraint ConnectionConstraint;
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSnapGridFlowModuleDatabaseItem {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, Category = Module, meta = (ToolTip = "逻辑关卡, 影响游戏逻辑，但不影响美术表现, 由策划编辑制作"))
    TSoftObjectPtr<UWorld> LogicLevel;
    
    UPROPERTY(EditAnywhere, Category = Module, meta = (ToolTip = "表现关卡, 影响美术表现、动态关卡生成, 由美术编辑制作"))
    TSoftObjectPtr<UWorld> Level;

    UPROPERTY(EditAnywhere, Category = Module)
    FName Category = "Room";

    UPROPERTY(EditAnywhere, Category = Module)
    bool bAllowRotation = true;

    /**
     * How often do you want this to be selected?  0.0 for least preference, 1.0 for most preference.  Specify a value from 0.0 to 1.0
     */
    UPROPERTY(EditAnywhere, Category = Module, Meta=(ClampMin="0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float SelectionWeight = 1.0f;
    
    UPROPERTY(VisibleAnywhere, Category = Module)
    FBox ModuleBounds;

    UPROPERTY(VisibleAnywhere, Category = Module)
    FIntVector NumChunks = FIntVector(1, 1, 1);
    
    UPROPERTY(VisibleAnywhere, Category = Module)
    TArray<FSnapGridFlowModuleDatabaseConnectionInfo> Connections;

    UPROPERTY(VisibleAnywhere, Category = Module)
    TMap<TSoftObjectPtr<UPlaceableMarkerAsset>, int32> AvailableMarkers;
    
    UPROPERTY()
    TArray<FSGFModuleAssembly> RotatedAssemblies;   // 4 Cached module assemblies rotated in 90 degree CW steps
};

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USnapGridFlowModuleDatabase : public UObject {
    GENERATED_BODY()
    public:
    UPROPERTY(EditAnywhere, Category = Module)
    USnapGridFlowModuleBoundsAsset* ModuleBoundsAsset;
    
    UPROPERTY(EditAnywhere, Category = Module)
    TArray<FSnapGridFlowModuleDatabaseItem> Modules;
};


