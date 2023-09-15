//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

enum class EDALatticeLinkDirection : uint8 {
    None      = 0,
    Left      = 1 << 0,
    Right     = 1 << 1,
    Front     = 1 << 2,
    Back      = 1 << 3,
    Top       = 1 << 4,
    Bottom    = 1 << 5
};

class DUNGEONARCHITECTRUNTIME_API FDALatticeUtils {
public:
    FORCEINLINE static EDALatticeLinkDirection GetOppositeDirection(const EDALatticeLinkDirection InDirection) {
        switch(InDirection) {
            case EDALatticeLinkDirection::Left: return EDALatticeLinkDirection::Right;
            case EDALatticeLinkDirection::Right: return EDALatticeLinkDirection::Left;
            case EDALatticeLinkDirection::Front: return EDALatticeLinkDirection::Back;
            case EDALatticeLinkDirection::Back: return EDALatticeLinkDirection::Front;
            case EDALatticeLinkDirection::Top: return EDALatticeLinkDirection::Bottom;
            case EDALatticeLinkDirection::Bottom: return EDALatticeLinkDirection::Top;
            default: return EDALatticeLinkDirection::None;
        }
    }
};

class DUNGEONARCHITECTRUNTIME_API FDALatticeBlockConnections {
public:
    void RegisterDirection(EDALatticeLinkDirection Direction) {
        DirectionMask |= static_cast<uint8>(Direction);
    }
    void Clear() {
       DirectionMask = 0; 
    }

    uint32 GetDirectionMask() const { return DirectionMask; }
    TArray<EDALatticeLinkDirection> GetConnectionDirections() const;
    
    
private:
    // Contains a bitmask of directions represented by EDALatticeLinkDirection
    uint32 DirectionMask = 0;
};

struct DUNGEONARCHITECTRUNTIME_API FDALatticeBlock {
    FIntVector Coords;
    
};


class DUNGEONARCHITECTRUNTIME_API FDALatticeWorld {
    
};

