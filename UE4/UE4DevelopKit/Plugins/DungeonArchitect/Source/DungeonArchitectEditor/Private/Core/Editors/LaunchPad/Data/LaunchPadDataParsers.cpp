//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Data/LaunchPadDataParsers.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogLaunchPadDataParsers, Log, All);

TSharedPtr<FJsonObject> FLaunchPadDataParser::ParseJson(const FString& InJson) {
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(InJson);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid()) {
        UE_LOG(LogLaunchPadDataParsers, Warning, TEXT("JsonObjectStringToUStruct - Unable to parse json=[%s]"),
               *InJson);
        return nullptr;
    }
    return JsonObject;
}

