// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UE4DevelopKit/UE4DevelopKit.h"
#include "VMTest.generated.h"

struct VMTest_TimeScope
{
	FString ScopeName;
	int64 TimeStart;

	VMTest_TimeScope(const FString& InScopeName) : ScopeName(InScopeName)
	{
		TimeStart = FDateTime::UtcNow().GetTicks() / ETimespan::TicksPerMillisecond;
	}

	~VMTest_TimeScope()
	{
		const int64 DeltaTime = FDateTime::UtcNow().GetTicks() / ETimespan::TicksPerMillisecond - TimeStart;
		UE_LOG(LogUDK, Log, TEXT("%s use time:%lld ms"), *ScopeName, DeltaTime);
	}
};

#define QUICK_SCOPE_TIME(InScopeName) VMTest_TimeScope TempTimeScope(#InScopeName)

UCLASS()
class UE4DEVELOPKIT_API AVMTest : public AActor
{
	GENERATED_BODY()

public:
	AVMTest();

	UPROPERTY(EditAnywhere)
	bool bNativizedMode = false;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetInLoops();

	void RunBpNativizedWrap();
	
	void RunBpWrap();

	void RunCppWrap();

	void RunLuaWrap();
	
	UFUNCTION(BlueprintImplementableEvent)
	int32 RunBp();

	int32 RunCpp();

	int32 RunLua();

protected:
	virtual void BeginPlay() override;
};
