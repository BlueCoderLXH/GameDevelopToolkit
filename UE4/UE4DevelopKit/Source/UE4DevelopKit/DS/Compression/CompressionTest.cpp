// Fill out your copyright notice in the Description page of Project Settings.
#include "CompressionTest.h"

void FCompressionPerformanceTest::DoTest(ENetworkCompressionType CompressionType, uint8* DataArray, int32 Size)
{	
	FMemory::Memzero(CompressBuffer, TestBufferLength);
	FMemory::Memzero(UnCompressBuffer, TestBufferLength);
	
	FNetCompressionDataWrap InData = { DataArray, Size * 8, Size };
	FNetCompressionDataWrap EncodeData = { CompressBuffer, TestBufferLength * 8, TestBufferLength };
	FNetCompressionDataWrap DecodeData = { UnCompressBuffer, Size * 8, Size };

	const FString SourceDataStr = InData.ToString();

	const TSharedPtr<FNetCompressionInterface>& CompressionHandler = TestData[(uint8)CompressionType].CompressionHandler;
	
	const int64 EncodeStartTime = FPlatformTime::Cycles64();
	CompressionHandler->Encode(InData, EncodeData);
	TestData[CompressionType].TotalEncodeTime += (FPlatformTime::Cycles64() - EncodeStartTime) / 1000.0f;
	
	const FString EncodeDataStr = EncodeData.ToString();

	const int64 DecodeStartTime = FPlatformTime::Cycles64();
	CompressionHandler->Decode(EncodeData, DecodeData);
	TestData[CompressionType].TotalDecodeTime += (FPlatformTime::Cycles64() - DecodeStartTime) / 1000.0f;

	const FString DecodeDataStr = DecodeData.ToString();

	const bool bIsResultOk = (SourceDataStr == DecodeDataStr);
	if (bIsResultOk)
	{
		TestData[CompressionType].TotalRatio += CompressionHandler->GetRatio() * 100;
		TestData[CompressionType].TestCount++;
	}

	TestData[CompressionType].bTestResult &= bIsResultOk;
}

ACompressionTest::ACompressionTest()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACompressionTest::BeginPlay()
{
	Super::BeginPlay();
	
	uint8* DataArray1 = new uint8[DataArrayCount];
	const int32 DataArray1Size = DataArrayCount;
	FMemory::Memzero(DataArray1, DataArray1Size);

	const int32 RandomSeed = FPlatformTime::Cycles();
	FMath::RandInit(RandomSeed);
	
	FCompressionPerformanceTest PerformanceTest;
	PerformanceTest.BeginTest();
	for (int32 i = 0; i < TestTimes; i++)
	{
		const uint8 TestMin = FMath::RandRange(TestRange.Min, TestRange.Max / 2);
		const uint8 TestMax = FMath::RandRange(TestRange.Max / 2 + 1, TestRange.Max);
		for (int32 j = 0; j < DataArray1Size; j++)
		{
			const uint8 RandomData = FMath::RandRange(TestMin, TestMax);
			DataArray1[j] = RandomData;
		}
	
		PerformanceTest.DoTest(HuffmanCode, DataArray1, DataArray1Size);
		PerformanceTest.DoTest(LZW, DataArray1, DataArray1Size);
		PerformanceTest.DoTest(LZ4, DataArray1, DataArray1Size);
		PerformanceTest.DoTest(Zlib, DataArray1, DataArray1Size);
		PerformanceTest.DoTest(Gzip, DataArray1, DataArray1Size);
	}
	PerformanceTest.EndTest();
	
	delete[] DataArray1;
	DataArray1 = nullptr;
}