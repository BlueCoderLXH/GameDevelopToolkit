//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/AppModes/FlowEdAppModeBase.h"

#include "Core/Editors/SnapMapEditor/SnapMapEditor.h"

FSnapMapEdAppModeBase::FSnapMapEdAppModeBase(FName InModeName)
    : FApplicationMode(InModeName, FSnapMapEditor::GetLocalizedMode) {
}

void FSnapMapEdAppModeBase::Tick(float DeltaTime) {

}

void FSnapMapEdAppModeBase::OnAssetSave() {

}

