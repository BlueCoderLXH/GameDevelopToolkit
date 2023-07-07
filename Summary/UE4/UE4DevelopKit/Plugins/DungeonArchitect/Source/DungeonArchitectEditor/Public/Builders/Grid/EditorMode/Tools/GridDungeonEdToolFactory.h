//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdTool.h"

class FGridDungeonEdToolFactory {
public:
    static TSharedPtr<FGridDungeonEdTool> Create(const FDungeonEdToolID& ToolType,
                                                 UGridDungeonEdModeHandler* ModeHandler);
};

