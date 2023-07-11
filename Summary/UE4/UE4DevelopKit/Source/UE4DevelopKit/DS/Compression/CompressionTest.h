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

struct FCompressionPerformanceTestData
{
	TSharedPtr<FNetCompressionInterface> CompressionHandler;
	
	double TotalRatio = 0.f;
	double TotalEncodeTime = 0.f;
	double TotalDecodeTime = 0.f;
	uint32 TestCount = 0;
	bool bTestResult = true;

	FString ToString() const
	{
		if (bTestResult)
		{
			return FString::Printf(TEXT("%-7s\t\tRatio:%.2f%%\t\tEncodeTime:%.2fms\t\tDecodeTime:%.2fms"),
				*CompressionHandler->GetName(), TotalRatio / TestCount, TotalEncodeTime / TestCount, TotalDecodeTime / TestCount);
		}
		return FString::Printf(TEXT("%s Wrong"), *CompressionHandler->GetName());
	}
};

struct FCompressionPerformanceTest
{
	static constexpr int32 TestBufferLength = 2048;
	
	uint8 CompressBuffer[TestBufferLength];
	uint8 UnCompressBuffer[TestBufferLength];

	FCompressionPerformanceTestData TestData[ENetworkCompressionType::Max];

	void BeginTest()
	{
		for (uint8 CompressionType = ENetworkCompressionType::Min; CompressionType < ENetworkCompressionType::Max; CompressionType++)
		{
			TestData[CompressionType].CompressionHandler = FNetCompressionFactory::New((ENetworkCompressionType)CompressionType);
			TestData[CompressionType].TotalRatio = 0.f;
			TestData[CompressionType].TotalEncodeTime = 0.f;
			TestData[CompressionType].TotalDecodeTime = 0.f;
			TestData[CompressionType].TestCount = 0;
			TestData[CompressionType].bTestResult = true;
		}
	}
	
	void DoTest(ENetworkCompressionType CompressionType, uint8* DataArray, int32 Size);

	void EndTest()
	{
		UE_LOG(LogNetCompression, Log, TEXT("---------------------------------------------------------------------------"));
		for (uint8 Index = ENetworkCompressionType::Min; Index < ENetworkCompressionType::Max; Index++)
		{
			const FString RetStr = TestData[Index].ToString();
			if (TestData[Index].bTestResult)
			{
				UE_LOG(LogNetCompression, Log, TEXT("%s"), *RetStr);
			}
			else
			{
				UE_LOG(LogNetCompression, Error, TEXT("%s"), *RetStr);
			}
			TestData[Index].CompressionHandler.Reset();
		}
	}
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
