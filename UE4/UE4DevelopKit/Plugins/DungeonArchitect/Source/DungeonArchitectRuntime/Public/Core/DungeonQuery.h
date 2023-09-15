//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "DungeonQuery.generated.h"

class UDungeonConfig;
class UDungeonModel;

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UDungeonQueryUserState : public UObject {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void SetInt(const FName& Name, int32 Value);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void SetBool(const FName& Name, bool Value);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void SetFloat(const FName& Name, float Value);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void SetVector(const FName& Name, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void SetString(const FName& Name, const FString& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    int32 GetInt(const FName& Name);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    bool GetBool(const FName& Name);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    float GetFloat(const FName& Name);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    FVector GetVector(const FName& Name);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    FString GetString(const FName& Name);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void ClearAllState();

private:
    UPROPERTY()
    TMap<FName, FString> UserData;
};

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UDungeonQuery : public UObject {
    GENERATED_BODY()
public:
    UDungeonQuery();

    void Initialize(UDungeonConfig* InConfig, UDungeonModel* InModel, const FTransform& InDungeonTransform);

public:
    UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
    UDungeonQueryUserState* UserState;

protected:
    virtual void InitializeImpl(UDungeonConfig* InConfig, UDungeonModel* InModel) {
    }

protected:
    FTransform DungeonTransform;
};

