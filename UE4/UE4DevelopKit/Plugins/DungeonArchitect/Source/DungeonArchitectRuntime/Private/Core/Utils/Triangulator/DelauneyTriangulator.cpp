//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/Triangulator/DelauneyTriangulator.h"


void DelauneyTriangulator::AddPoint(const FVector2D& Point) {
    Points.Add(Point);
}

void DelauneyTriangulator::Triangulate() {
    PerformTriangulation();
}

