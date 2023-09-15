//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SnapMap/EditorMode/Tools/SnapMapEdTool.h"

#include "Builders/SnapMap/EditorMode/SnapMapEdModeHandler.h"
#include "Builders/SnapMap/EditorMode/SnapMapEdModeRenderer.h"

FName FSnapMapEdTool::ToolFamily = "SnapMap";

FSnapMapEdTool::FSnapMapEdTool(USnapMapEdModeHandler* InModeHandler) : ModeHandler(InModeHandler) {
}

void FSnapMapEdTool::Initialize() {
}

void FSnapMapEdTool::Update(const FSceneView* View, FViewport* Viewport) {

}

void FSnapMapEdTool::Destroy() {
}

void FSnapMapEdTool::OnUndo() {
}

void FSnapMapEdTool::OnRedo() {
}

void FSnapMapEdTool::AddReferencedObjects(FReferenceCollector& Collector) {
}

UObject* FSnapMapEdTool::GetToolModel() const {
    return nullptr;
}

TSharedPtr<FSnapMapEdModeRenderer> FSnapMapEdTool::GetSnapMapRenderer() {
    if (!ModeHandler) {
        return nullptr;
    }
    return StaticCastSharedPtr<FSnapMapEdModeRenderer>(ModeHandler->GetRenderer());
}

FName FSnapMapEdTool::GetToolFamily() const {
    return ToolFamily;
}

