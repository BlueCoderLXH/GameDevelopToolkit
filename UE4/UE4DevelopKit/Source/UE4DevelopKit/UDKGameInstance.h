// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UDKGameInstance.generated.h"

UCLASS()
class UE4DEVELOPKIT_API UUDKGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

private:
	void InitLua();
};
