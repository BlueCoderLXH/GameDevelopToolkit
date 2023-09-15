//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/EditorMode/DungeonEdModeHandler.h"

#include "Core/Common/Utils/DungeonEditorUtils.h"
#include "Core/Dungeon.h"
#include "Core/LevelEditor/EditorMode/DungeonEdModeRenderer.h"
#include "Core/LevelEditor/EditorMode/IDungeonEdTool.h"

#include "Editor/LevelEditor/Public/SLevelViewport.h"
#include "EditorViewportClient.h"
#include "SceneManagement.h"

void UDungeonEdModeHandler::Enter() {
    InitializeSettings();

    Renderer = CreateRenderer();
}

void UDungeonEdModeHandler::Exit() {
    if (ActiveTool.IsValid()) {
        ActiveTool->Destroy();
        ActiveTool = nullptr;
    }
    if (Renderer.IsValid()) {
        Renderer->Destroy();
        Renderer = nullptr;
    }
}

void UDungeonEdModeHandler::Undo() {
    if (ActiveTool.IsValid()) {
        ActiveTool->OnUndo();
    }

    RebuildDungeon();
}

void UDungeonEdModeHandler::Redo() {
    if (ActiveTool.IsValid()) {
        ActiveTool->OnRedo();
    }

    RebuildDungeon();
}

bool UDungeonEdModeHandler::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key,
                                     EInputEvent Event) {
    if (ActiveTool.IsValid()) {
        return ActiveTool->InputKey(ViewportClient, Viewport, Key, Event);
    }

    return false;
}

void UDungeonEdModeHandler::UpdateFrame(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) {
    OnUpdate(View, Viewport);
    OnRender(View, Viewport, PDI);
}

void UDungeonEdModeHandler::OnUpdate(const FSceneView* View, FViewport* Viewport) {
    if (ActiveTool.IsValid()) {
        ActiveTool->Update(View, Viewport);
    }
}

void UDungeonEdModeHandler::OnRender(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) {
    if (Renderer.IsValid()) {
        Renderer->Render(View, Viewport, PDI, ActiveTool);
    }
    if (ActiveTool.IsValid()) {
        ActiveTool->Render(View, Viewport, PDI);
    }
}

/*
void UDungeonEdModeHandler::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (ActiveTool.IsValid()) {
		ActiveTool->AddReferencedObjects(Collector);
	}
}
*/

void UDungeonEdModeHandler::ApplyBrush(FEditorViewportClient* ViewportClient) {
    if (ActiveTool.IsValid()) {
        ActiveTool->ApplyBrush(ViewportClient);
    }
}

void UDungeonEdModeHandler::InitializeSettings() {
    ActiveDungeon = FDungeonEditorUtils::GetDungeonActorFromLevelViewport();
}

void UDungeonEdModeHandler::RebuildDungeon() {
    if (ActiveDungeon) {
        ActiveDungeon->BuildDungeon();
    }
}

void UDungeonEdModeHandler::OnDungeonSelectionChanged(ADungeon* pDungeon) {
    // TODO: Recreate handler and renderer here based on builder type

    ActiveDungeon = pDungeon;
    if (ActiveTool.IsValid()) {
        ActiveTool->Initialize();
    }
}

TSharedPtr<FDungeonEdModeRenderer> UDungeonEdModeHandler::GetRenderer() const {
    return Renderer;
}

ADungeon* UDungeonEdModeHandler::GetActiveDungeon() const {
    return ActiveDungeon;
}

void UDungeonEdModeHandler::SetActiveTool(TSharedPtr<IDungeonEdTool> NewTool) {
    if (ActiveTool.IsValid()) {
        ActiveTool->Destroy();
    }
    ActiveTool = NewTool;
    if (ActiveTool.IsValid()) {
        ActiveTool->Initialize();
    }
}

