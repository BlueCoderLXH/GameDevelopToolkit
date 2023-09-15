//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class UEdGraph_Grammar;

class DUNGEONARCHITECTEDITOR_API IGraphLayout {
public:
    virtual void PerformLayout(UEdGraph_Grammar* Graph) = 0;
};

