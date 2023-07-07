//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

struct FSnapMapModuleDatabaseItem;

namespace SnapLib {
    enum class EFunctionType : uint8 {
        MoveToNode,
        BacktrackFromNode,
        AssignModule,
        RejectModule,
        TimeoutHalt
    };

    enum class EModuleRejectReason : uint8 {
        None,
        NoModuleAvailable,
        // No module with this category available
        NotEnoughDoorsAvailable,
        // Not enough doors available in the current module as required by the mission graph's outgoing links
        BoundsCollide,
        // This module's bounds collide with an existing placed room
        CannotBuildSubTree,
    };

    struct IPayload {
    };

    typedef TSharedPtr<IPayload> IPayloadPtr;

    struct FDiagStep {
        EFunctionType Function;
        IPayloadPtr Payload;
    };

    /////////////////////////////////////////////////////

    class DUNGEONARCHITECTRUNTIME_API FDiagnostics {
    public:
        FDiagnostics();
        void LogMoveToNode(const FGuid& InNodeId);
        void LogBacktrackFromNode(bool InbSuccess);
        void LogAssignModule(TSoftObjectPtr<UWorld> InModuleLevel, FBox InModuleBounds, const FTransform& InWorldTransform,
                             int32 InIncomingDoorIndex, int32 InRemoteDoorIndex, const FGuid& InIncomingDoorId,
                             const FGuid& InRemoteDoorId,
                             const FGuid& InNodeId, const FGuid& InRemoteNodeId, const FBox& InIncomingDoorBounds);
        void LogRejectModule(EModuleRejectReason InReason);
        void LogTimeoutHalt();

        void Clear();

        TArray<FDiagStep> Steps;
    };

    ////////////// Payload data structures //////////////
    struct FPayload_MoveToNode : IPayload {
        FGuid NodeId;
    };

    struct FPayload_BacktrackFromNode : IPayload {
        bool bSuccess;
    };

    struct FPayload_AssignModule : IPayload {
        TSoftObjectPtr<UWorld> ModuleLevel;
        FBox ModuleBounds;
        FTransform WorldTransform = FTransform::Identity;
        int32 IncomingDoorIndex = -1;
        int32 RemoteDoorIndex = -1;
        FGuid IncomingDoorId;
        FGuid RemoteDoorId;
        FGuid NodeId;
        FGuid RemoteNodeId;
        FBox IncomingDoorBounds;
    };

    struct FPayload_RejectModule : IPayload {
        EModuleRejectReason Reason;
    };

    struct FPayload_TimeoutHalt : IPayload {

    };
};

