//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "IDungeonArchitectRuntime.h"

#include "Core/Utils/EditorService/DungeonNonEditorFallbackService.h"

class FDungeonArchitectRuntime : public IDungeonArchitectRuntime {
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FDungeonArchitectRuntime, DungeonArchitectRuntime)

void FDungeonArchitectRuntime::StartupModule() {
    // Add the runtime fallback editor service module (if not already set by the editor)
    if (!IDungeonEditorService::Get().IsValid()) {
        IDungeonEditorService::Set(MakeShareable(new FDungeonNonEditorFallbackService));
    }
}


void FDungeonArchitectRuntime::ShutdownModule() {
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    IDungeonEditorService::Get().Reset();
}

