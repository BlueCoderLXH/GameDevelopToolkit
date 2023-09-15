//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/DungeonQuery.h"


UDungeonQuery::UDungeonQuery() {
    UserState = CreateDefaultSubobject<UDungeonQueryUserState>("UserState");
}

void UDungeonQuery::Initialize(UDungeonConfig* InConfig, UDungeonModel* InModel, const FTransform& InDungeonTransform) {
    this->DungeonTransform = InDungeonTransform;
    InitializeImpl(InConfig, InModel);
}

void UDungeonQueryUserState::SetInt(const FName& Name, int32 Value) {
    FString StrValue = FString::FromInt(Value);
    SetString(Name, StrValue);
}

void UDungeonQueryUserState::SetBool(const FName& Name, bool Value) {
    SetInt(Name, static_cast<int>(Value));
}

void UDungeonQueryUserState::SetFloat(const FName& Name, float Value) {
    FString StrValue = FString::SanitizeFloat(Value);
    SetString(Name, StrValue);
}

void UDungeonQueryUserState::SetVector(const FName& Name, const FVector& Value) {
    FString StrValue = Value.ToString();
    SetString(Name, StrValue);
}

void UDungeonQueryUserState::SetString(const FName& Name, const FString& Value) {
    FString& MapValue = UserData.FindOrAdd(Name);
    MapValue = Value;
}

int32 UDungeonQueryUserState::GetInt(const FName& Name) {
    FString StrValue = GetString(Name);
    return FCString::Atoi(*StrValue);
}

bool UDungeonQueryUserState::GetBool(const FName& Name) {
    int32 IntValue = GetInt(Name);
    return IntValue != 0;
}

float UDungeonQueryUserState::GetFloat(const FName& Name) {
    FString StrValue = GetString(Name);
    return FCString::Atof(*StrValue);
}

FVector UDungeonQueryUserState::GetVector(const FName& Name) {
    FString StrValue = GetString(Name);
    FVector Value;
    Value.InitFromString(StrValue);
    return Value;
}

FString UDungeonQueryUserState::GetString(const FName& Name) {
    FString* SearchResult = UserData.Find(Name);
    return SearchResult ? *SearchResult : "";
}

void UDungeonQueryUserState::ClearAllState() {
    UserData.Reset();
}

