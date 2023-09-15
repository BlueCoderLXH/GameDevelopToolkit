//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/EditorMode/DungeonEdModeHandlerFactory.h"

#include "Builders/Grid/EditorMode/GridDungeonEdModeHandler.h"
#include "Builders/Grid/GridDungeonBuilder.h"
#include "Builders/SnapMap/EditorMode/SnapMapEdModeHandler.h"
#include "Builders/SnapMap/SnapMapDungeonBuilder.h"

TSharedPtr<FDungeonEdModeHandlerFactory> FDungeonEdModeHandlerFactory::Instance;

void FDungeonEdModeHandlerFactory::StaticInit(void) {
    if (!Instance.IsValid()) {
        Instance = MakeShareable(new FDungeonEdModeHandlerFactory);
    }
}

TSharedPtr<FDungeonEdModeHandlerFactory> FDungeonEdModeHandlerFactory::Get() {
    if (!Instance.IsValid()) {
        Instance = MakeShareable(new FDungeonEdModeHandlerFactory);
    }
    return Instance;
}

void FDungeonEdModeHandlerFactory::Register() {
    // Register the default handlers
    Get()->RegisterHandler(UGridDungeonBuilder::StaticClass(), UGridDungeonEdModeHandler::StaticClass());
    Get()->RegisterHandler(USnapMapDungeonBuilder::StaticClass(), USnapMapEdModeHandler::StaticClass());
}

void FDungeonEdModeHandlerFactory::RegisterHandler(TSubclassOf<UDungeonBuilder> BuilderClass,
                                                   TSubclassOf<UDungeonEdModeHandler> HandlerClass) {
    BuilderToHandlerMapping.Add(BuilderClass, HandlerClass);
}

UDungeonEdModeHandler* FDungeonEdModeHandlerFactory::CreateHandler(TSubclassOf<UDungeonBuilder> BuilderClass,
                                                                   UObject* Outer
                                                                   /*= (UObject*)GetTransientPackage()*/) {
    if (!BuilderToHandlerMapping.Contains(BuilderClass)) {
        return nullptr;
    }

    TSubclassOf<UDungeonEdModeHandler> HandlerClass = BuilderToHandlerMapping[BuilderClass];
    return NewObject<UDungeonEdModeHandler>(Outer, HandlerClass);
}

