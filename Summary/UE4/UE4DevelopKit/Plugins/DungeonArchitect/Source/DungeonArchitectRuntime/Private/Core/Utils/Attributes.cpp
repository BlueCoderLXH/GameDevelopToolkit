//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/Attributes.h"


void FDAAttribute::Reset() {
    NumberValue.Reset();
    BoolValue.Reset();
    SizeValue.Reset();
    StringValue.Reset();
    StringArrayValue.Reset();
}


FDAAttribute FDAAttribute::ParseNumber(const FString& InSerializedText) {
    return FDAAttribute(FCString::Atof(*InSerializedText));
}

FDAAttribute FDAAttribute::ParseBool(const FString& InSerializedText) {
    return FDAAttribute(FCString::ToBool(*InSerializedText));
}

FDAAttribute FDAAttribute::ParseSize(const FString& InSerializedText) {
    float X = 0;
    float Y = 0;
    FString StrX, StrY;
    if (InSerializedText.Split(",", &StrX, &StrY)) {
        X = FCString::Atof(*StrX);
        Y = FCString::Atof(*StrY);
    }
    return FDAAttribute(FVector2D(X, Y));
}

FDAAttribute FDAAttribute::ParseVector(const FString& InSerializedText) {
    float X = 0;
    float Y = 0;
    float Z = 0;
    FString StrX, StrY, StrZ;
    FString Intermediate;
    if (InSerializedText.Split(",", &StrX, &Intermediate)) {
        if (Intermediate.Split(",", &StrY, &StrZ)) {
            X = FCString::Atof(*StrX);
            Y = FCString::Atof(*StrY);
            Z = FCString::Atof(*StrZ);
        }
    }
    return FDAAttribute(FVector(X, Y, Z));
}

FDAAttribute FDAAttribute::ParseString(const FString& InSerializedText) {
    return FDAAttribute(InSerializedText);
}

FDAAttribute FDAAttribute::ParseStringArray(const FString& InSerializedText) {
    TArray<FString> Array;
    //if (Array.Num() > 0) 
    {
        FString Paths = InSerializedText;
        FString Left;
        FString Right;
        while (Paths.Split(",", &Left, &Right, ESearchCase::CaseSensitive, ESearchDir::FromStart)) {
            if (Left.Len() > 0) {
                Array.Add(Left);
            }
            Paths = Right;
        }
        if (Paths.Len() > 0) {
            Array.Add(Paths);
        }
    }
    return FDAAttribute(Array);
}

void FDAAttributeList::Reset() {
    AttributesByNode.Reset();
}

bool FDAAttributeList::SetAttribute(const FString& FullPath, const FDAAttribute& InAttribute) {
    int32 DotIndex;
    FullPath.FindChar('.', DotIndex);
    if (DotIndex == INDEX_NONE) {
        return false;
    }
    FString NodeName = FullPath.Mid(0, DotIndex);
    FString VariableName = FullPath.Mid(DotIndex + 1);
    if (NodeName.Len() == 0 || VariableName.Len() == 0) {
        return false;
    }

    TMap<FString, FDAAttribute>& Attributes = AttributesByNode.FindOrAdd(NodeName);
    FDAAttribute& Attribute = Attributes.FindOrAdd(VariableName);
    Attribute = InAttribute;
    return true;
}

