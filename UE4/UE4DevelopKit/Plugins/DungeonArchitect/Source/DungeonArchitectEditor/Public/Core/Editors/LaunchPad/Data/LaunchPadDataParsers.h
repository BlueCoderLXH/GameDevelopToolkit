//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "JsonUtilities/Public/JsonObjectConverter.h"

struct FLaunchPadCategories;
class FJsonObject;

class FLaunchPadDataParser {
public:
    static TSharedPtr<FJsonObject> ParseJson(const FString& InJson);

    template <typename T>
    static bool Parse(TSharedPtr<FJsonObject> InJsonObject, T& OutCategories) {
        return FJsonObjectConverter::JsonObjectToUStruct(InJsonObject.ToSharedRef(), &OutCategories);
    }
};

