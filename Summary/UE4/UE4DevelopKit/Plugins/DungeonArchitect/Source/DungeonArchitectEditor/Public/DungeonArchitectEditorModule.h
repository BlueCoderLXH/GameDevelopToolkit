//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/ThemeEditor/IDungeonArchitectThemeEditor.h"

#include "AssetTypeCategories.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Toolkits/IToolkit.h"

/**
 * The public interface to this module
 */
class IDungeonArchitectEditorModule : public IModuleInterface {

public:

    /**
     * Singleton-like access to this module's interface.  This is just for convenience!
     * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
     *
     * @return Returns singleton instance, loading the module on demand if needed
     */
    static IDungeonArchitectEditorModule& Get() {
        return FModuleManager::LoadModuleChecked<IDungeonArchitectEditorModule>("DungeonArchitectEditor");
    }

    /**
     * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
     *
     * @return True if the module is loaded and ready to use
     */
    static bool IsAvailable() {
        return FModuleManager::Get().IsModuleLoaded("DungeonArchitectEditor");
    }

    virtual EAssetTypeCategories::Type GetDungeonAssetCategoryBit() const = 0;
};

