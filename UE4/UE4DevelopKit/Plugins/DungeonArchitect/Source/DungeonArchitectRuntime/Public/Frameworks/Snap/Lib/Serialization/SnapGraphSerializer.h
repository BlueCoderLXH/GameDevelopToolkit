//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Snap/Lib/Serialization/ConnectionSerialization.h"
#include "Frameworks/Snap/Lib/SnapLibrary.h"

template<typename TModuleData>
class DUNGEONARCHITECTRUNTIME_API TSnapGraphSerializer {
public:
    static void Serialize(const TArray<SnapLib::FModuleNodePtr>& InNodes, TArray<TModuleData>& OutModuleInstances, TArray<struct FSnapConnectionInstance>& OutConnections) {
        OutConnections.Reset();
        OutModuleInstances.Reset();

        TSet<SnapLib::FModuleNodePtr> Visited;
        for (SnapLib::FModuleNodePtr Node : InNodes) {
            SerializeImpl(Node, OutModuleInstances, OutConnections, Visited);
        }
    }

private:
    static FSnapConnectionInstance ReverseConnection(const FSnapConnectionInstance& InConnection) {
        FSnapConnectionInstance Connection;
        Connection.ModuleA = InConnection.ModuleB;
        Connection.DoorA = InConnection.DoorB;
        Connection.ModuleB = InConnection.ModuleA;
        Connection.DoorB = InConnection.DoorA;
        Connection.WorldTransform = InConnection.WorldTransform; // TODO: Rotate along Z by 180
        Connection.bReverse = true;
        return Connection;
    }
    
    static void SerializeImpl(SnapLib::FModuleNodePtr InNode, TArray<TModuleData>& OutModuleInstances,
                       TArray<FSnapConnectionInstance>& OutConnections, TSet<SnapLib::FModuleNodePtr>& Visited) {
        if (!InNode.IsValid() || Visited.Contains(InNode)) {
            return;
        }
        Visited.Add(InNode);

        TModuleData ModuleData;
        ModuleData.ModuleInstanceId = InNode->ModuleInstanceId;
        ModuleData.WorldTransform = InNode->WorldTransform;
        ModuleData.Level = InNode->ModuleDBItem->GetLevel();
        ModuleData.Category = InNode->ModuleDBItem->GetCategory();
        ModuleData.ModuleBounds = InNode->ModuleDBItem->GetBounds();
        OutModuleInstances.Add(ModuleData);

        // Save the node's outgoing links in the connection array and traverse those links
        for (SnapLib::FModuleDoorPtr Door : InNode->Outgoing) {
            if (Door.IsValid() && Door->ConnectedDoor.IsValid() && Door->ConnectedDoor->Owner.IsValid()) {
                FSnapConnectionInstance Connection;
                Connection.ModuleA = InNode->ModuleInstanceId;
                Connection.DoorA = Door->ConnectionId;

                SnapLib::FModuleDoorPtr OtherDoor = Door->ConnectedDoor;
                Connection.ModuleB = OtherDoor->Owner->ModuleInstanceId;
                Connection.DoorB = OtherDoor->ConnectionId;

                Connection.WorldTransform = Door->LocalTransform * InNode->WorldTransform;

                Connection.bReverse = false;

                OutConnections.Add(Connection);
                OutConnections.Add(ReverseConnection(Connection));

                SerializeImpl(OtherDoor->Owner, OutModuleInstances, OutConnections, Visited);
            }
        } 
    }
};

