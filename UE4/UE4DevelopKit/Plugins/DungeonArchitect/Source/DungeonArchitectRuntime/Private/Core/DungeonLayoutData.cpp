//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/DungeonLayoutData.h"


void FDungeonLayoutData::AddQuadItem(const FVector2D& InLocation, const FVector2D& InSize, int32 ItemId,
                                     int32 ItemCategory) {
    FDungeonLayoutDataBlockItem Item;
    Item.ItemId = ItemId;
    Item.ItemCategory = ItemCategory;

    FVector2D P00(InLocation.X, InLocation.Y);
    FVector2D P10(InLocation.X + InSize.X, InLocation.Y);
    FVector2D P11(InLocation.X + InSize.X, InLocation.Y + InSize.Y);
    FVector2D P01(InLocation.X, InLocation.Y + InSize.Y);

    Item.Outline = {P00, P10, P11, P01};
    Item.FillTriangles = {
        P00, P10, P11,
        P00, P11, P01
    };
    LayoutItems.Add(Item);
}

