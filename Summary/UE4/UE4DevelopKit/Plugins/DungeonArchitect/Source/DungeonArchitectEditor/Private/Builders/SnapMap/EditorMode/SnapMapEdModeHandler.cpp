//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SnapMap/EditorMode/SnapMapEdModeHandler.h"

#include "Builders/SnapMap/EditorMode/SnapMapEdModeRenderer.h"
#include "Builders/SnapMap/EditorMode/UI/SSnapMapEditor.h"

DEFINE_LOG_CATEGORY(SnapMapEdModeHandlerLog);

TSharedPtr<FDungeonEdModeRenderer> USnapMapEdModeHandler::CreateRenderer() {
    return MakeShareable(new FSnapMapEdModeRenderer(this));
}

void USnapMapEdModeHandler::OnUpdate(const FSceneView* View, FViewport* Viewport) {
    UDungeonEdModeHandler::OnUpdate(View, Viewport);
}

void USnapMapEdModeHandler::SetActiveTool(TSharedPtr<IDungeonEdTool> NewTool) {
    UDungeonEdModeHandler::SetActiveTool(NewTool);
}

TSharedPtr<SWidget> USnapMapEdModeHandler::CreateToolkitWidget() {
    return SNew(SSnapMapEditor);
}

