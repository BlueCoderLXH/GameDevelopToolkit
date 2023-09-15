//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolFactory.h"

#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolBorder.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolPaint.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolRectangle.h"

TSharedPtr<FGridDungeonEdTool> FGridDungeonEdToolFactory::Create(const FDungeonEdToolID& ToolType,
                                                                 UGridDungeonEdModeHandler* ModeHandler) {
    if (ToolType == FGridDungeonEdToolPaint::ToolID) {
        return MakeShareable(new FGridDungeonEdToolPaint(ModeHandler));
    }
    if (ToolType == FGridDungeonEdToolRectangle::ToolID) {
        return MakeShareable(new FGridDungeonEdToolRectangle(ModeHandler));
    }
    if (ToolType == FGridDungeonEdToolBorder::ToolID) {
        return MakeShareable(new FGridDungeonEdToolBorder(ModeHandler));
    }
    return nullptr;
}

