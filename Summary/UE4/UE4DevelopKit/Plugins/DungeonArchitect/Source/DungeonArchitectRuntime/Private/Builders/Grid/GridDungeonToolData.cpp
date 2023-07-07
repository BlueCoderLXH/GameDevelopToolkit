//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/GridDungeonToolData.h"


bool operator==(const FGridToolRectStrokeData& A, const FGridToolRectStrokeData& B) {
    return A.Rectangle == B.Rectangle; // && A.CellType == B.CellType;
}

bool operator==(const FGridToolPaintStrokeData& A, const FGridToolPaintStrokeData& B) {
    return A.Location == B.Location; // && A.CellType == B.CellType;
}

