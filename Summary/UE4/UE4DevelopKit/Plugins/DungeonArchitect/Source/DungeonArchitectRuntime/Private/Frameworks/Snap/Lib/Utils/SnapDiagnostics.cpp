//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Snap/Lib/Utils/SnapDiagnostics.h"


SnapLib::FDiagnostics::FDiagnostics() {

}

void SnapLib::FDiagnostics::LogMoveToNode(const FGuid& InNodeId) {
    TSharedPtr<FPayload_MoveToNode> Payload = MakeShareable(new FPayload_MoveToNode);
    Payload->NodeId = InNodeId;

    FDiagStep& Step = Steps.AddDefaulted_GetRef();
    Step.Function = EFunctionType::MoveToNode;
    Step.Payload = Payload;
}

void SnapLib::FDiagnostics::LogBacktrackFromNode(bool InbSuccess) {
    TSharedPtr<FPayload_BacktrackFromNode> Payload = MakeShareable(new FPayload_BacktrackFromNode);
    Payload->bSuccess = InbSuccess;

    FDiagStep& Step = Steps.AddDefaulted_GetRef();
    Step.Function = EFunctionType::BacktrackFromNode;
    Step.Payload = Payload;
}

void SnapLib::FDiagnostics::LogAssignModule(TSoftObjectPtr<UWorld> InModuleLevel, FBox InModuleBounds,
                                                       const FTransform& InWorldTransform,
                                                       int32 InIncomingDoorIndex, int32 InRemoteDoorIndex,
                                                       const FGuid& InIncomingDoorId, const FGuid& InRemoteDoorId,
                                                       const FGuid& InNodeId, const FGuid& InRemoteNodeId,
                                                       const FBox& InIncomingDoorBounds) {
    TSharedPtr<FPayload_AssignModule> Payload = MakeShareable(new FPayload_AssignModule);
    Payload->ModuleLevel = InModuleLevel;
    Payload->ModuleBounds = InModuleBounds;
    Payload->WorldTransform = InWorldTransform;
    Payload->IncomingDoorId = InIncomingDoorId;
    Payload->RemoteDoorId = InRemoteDoorId;
    Payload->IncomingDoorIndex = InIncomingDoorIndex;
    Payload->RemoteDoorIndex = InRemoteDoorIndex;
    Payload->NodeId = InNodeId;
    Payload->RemoteNodeId = InRemoteNodeId;
    Payload->IncomingDoorBounds = InIncomingDoorBounds;

    FDiagStep& Step = Steps.AddDefaulted_GetRef();
    Step.Function = EFunctionType::AssignModule;
    Step.Payload = Payload;
}

void SnapLib::FDiagnostics::LogRejectModule(EModuleRejectReason InReason) {
    TSharedPtr<FPayload_RejectModule> Payload = MakeShareable(new FPayload_RejectModule);
    Payload->Reason = InReason;

    FDiagStep& Step = Steps.AddDefaulted_GetRef();
    Step.Function = EFunctionType::RejectModule;
    Step.Payload = Payload;
}

void SnapLib::FDiagnostics::LogTimeoutHalt() {
    TSharedPtr<FPayload_TimeoutHalt> Payload = MakeShareable(new FPayload_TimeoutHalt);

    FDiagStep& Step = Steps.AddDefaulted_GetRef();
    Step.Function = EFunctionType::TimeoutHalt;
    Step.Payload = Payload;
}

void SnapLib::FDiagnostics::Clear() {
    Steps.Reset();
}

