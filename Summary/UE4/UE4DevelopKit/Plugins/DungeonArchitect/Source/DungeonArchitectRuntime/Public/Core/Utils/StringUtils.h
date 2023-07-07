//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

// A custom key compare function so TMaps don't map to case sensitive strings
template <typename ValueType>
struct TCaseStringHasher : BaseKeyFuncs<TPair<FString, ValueType>, FString, false> {
    typedef typename TTypeTraits<FString>::ConstPointerType KeyInitType;
    typedef const TPairInitializer<typename TTypeTraits<FString>::ConstInitType, typename TTypeTraits<ValueType
                                   >::ConstInitType>& ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element) {
        return Element.Key;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B) {
        return A.Equals(B, ESearchCase::CaseSensitive);
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType Key) {
        return GetTypeHash(Key);
    }
};


class DUNGEONARCHITECTRUNTIME_API FDAStringUtils {
public:
    static TArray<FString> Split(const FString& InText, const FString& InSeparator);
};

