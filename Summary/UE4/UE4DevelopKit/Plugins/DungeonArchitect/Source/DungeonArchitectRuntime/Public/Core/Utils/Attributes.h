//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

template <typename T>
class TDAAttributeEntry {
public:
    TDAAttributeEntry() {
    }

    TDAAttributeEntry(const T& InValue) { Set(InValue); }

    void Set(const T& InValue) {
        Value = InValue;
        bIsSet = true;
    }

    T Get() const { return Value; }

    bool Get(T& OutValue) {
        if (bIsSet) {
            OutValue = Value;
        }
        return bIsSet;
    }

    bool IsSet() const { return bIsSet; }
    void Reset() { bIsSet = false; }

private:
    bool bIsSet = false;
    T Value;
};


struct DUNGEONARCHITECTRUNTIME_API FDAAttribute {
    FDAAttribute() {
    }

    FDAAttribute(float InValue) : NumberValue(InValue) {
    }

    FDAAttribute(int32 InValue) : NumberValue(InValue) {
    }

    FDAAttribute(bool InValue) : BoolValue(InValue) {
    }

    FDAAttribute(const FVector2D& InValue) : SizeValue(InValue) {
    }

    FDAAttribute(const FVector& InValue) : VectorValue(InValue) {
    }

    FDAAttribute(const FString& InValue) : StringValue(InValue) {
    }

    FDAAttribute(const TArray<FString>& InValue) : StringArrayValue(InValue) {
    }

    void Reset();


    static FDAAttribute ParseNumber(const FString& InSerializedText);
    static FDAAttribute ParseBool(const FString& InSerializedText);
    static FDAAttribute ParseSize(const FString& InSerializedText);
    static FDAAttribute ParseVector(const FString& InSerializedText);
    static FDAAttribute ParseString(const FString& InSerializedText);
    static FDAAttribute ParseStringArray(const FString& InSerializedText);

public:
    TDAAttributeEntry<float> NumberValue;
    TDAAttributeEntry<bool> BoolValue;
    TDAAttributeEntry<FVector2D> SizeValue;
    TDAAttributeEntry<FVector> VectorValue;
    TDAAttributeEntry<FString> StringValue;
    TDAAttributeEntry<TArray<FString>> StringArrayValue;
};

class DUNGEONARCHITECTRUNTIME_API FDAAttributeList {
public:
    void Reset();
    bool SetAttribute(const FString& FullPath, const FDAAttribute& InAttribute);

public:
    TMap<FString, TMap<FString, FDAAttribute>> AttributesByNode;
};

