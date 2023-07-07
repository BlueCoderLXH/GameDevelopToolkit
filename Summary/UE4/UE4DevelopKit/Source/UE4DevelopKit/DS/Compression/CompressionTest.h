// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetCompressionInterface.h"
#include "CompressionTest.generated.h"

USTRUCT(BlueprintType)
struct FCompressTestRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint8 Min = 0;

	UPROPERTY(EditAnywhere)
	uint8 Max = 255;

	FString ToString() const
	{
		FString Ret;
		Ret.Appendf(TEXT("[%d, %d]"), Min, Max);
		return Ret;
	}
};

struct FCompressionTestHelper
{
	static constexpr int32 TestBufferLength = 2048;
	
	uint8 CompressBuffer[TestBufferLength];
	uint8 UnCompressBuffer[TestBufferLength];
	
	void DoTest(ENetworkCompressionType CompressionType, uint8* DataArray, int32 Size);
};

UCLASS(Blueprintable, BlueprintType)
class UE4DEVELOPKIT_API ACompressionTest : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 DataArrayCount = 5;

	UPROPERTY(EditAnywhere)
	int32 TestTimes = 1;

	UPROPERTY(EditAnywhere)
	FCompressTestRange TestRange;
	
	ACompressionTest();

protected:
	virtual void BeginPlay() override;
};
