// Fill out your copyright notice in the Description page of Project Settings.
#include "CompressionTest.h"

void FCompressionTestHelper::DoTest(ENetworkCompressionType CompressionType, uint8* DataArray, int32 Size)
{
	FMemory::Memzero(CompressBuffer, TestBufferLength);
	FMemory::Memzero(UnCompressBuffer, TestBufferLength);
	
	FNetCompressionDataWrap InData = { DataArray, Size * 8, Size };
	FNetCompressionDataWrap EncodeData = { CompressBuffer, TestBufferLength * 8, TestBufferLength };
	FNetCompressionDataWrap DecodeData = { UnCompressBuffer, Size * 8, Size };

	const FString SourceDataStr = InData.ToString();

	const TSharedPtr<FNetCompressionInterface> CompressionHandler = FNetCompressionFactory::New(CompressionType);
	if (!CompressionHandler) return;

	FString EncodeDataStr;
	if (CompressionHandler->Encode(InData, EncodeData))
	{
		EncodeDataStr = EncodeData.ToString();
	}

	FString DecodeDataStr;
	if (CompressionHandler->Decode(EncodeData, DecodeData))
	{
		DecodeDataStr = DecodeData.ToString();
	}

	const bool bIsResultOk = (SourceDataStr == DecodeDataStr);
	if (bIsResultOk)
	{
		UE_LOG(LogNetCompression, Log, TEXT("%s result:Right ratio:%.2f%%"), *(CompressionHandler->GetName()), CompressionHandler->GetRatio() * 100);
	}
	else
	{
		UE_LOG(LogNetCompression, Log, TEXT("SourceDataStr: %s"), *SourceDataStr);
		UE_LOG(LogNetCompression, Log, TEXT("EncodeDataStr: %s"), *EncodeDataStr);
		UE_LOG(LogNetCompression, Log, TEXT("DecodeDataStr: %s"), *DecodeDataStr);
		
		UE_LOG(LogNetCompression, Error, TEXT("%s result:Wrong ratio:%.2f%%"), *(CompressionHandler->GetName()), CompressionHandler->GetRatio() * 100);
	}
}

ACompressionTest::ACompressionTest()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACompressionTest::BeginPlay()
{
	Super::BeginPlay();

	// uint8 DataArray1[] = {
	// 	'A', 'B', 'A', 'B', 'A', 'B', 'A'
	// };
	// constexpr int32 DataArray1Size = sizeof(DataArray1) / sizeof(uint8);
	
	uint8* DataArray1 = new uint8[DataArrayCount];
	const int32 DataArray1Size = DataArrayCount;
	FMemory::Memzero(DataArray1, DataArray1Size);
	
	FCompressionTestHelper TestHelper;
	for (int32 i = 0; i < TestTimes; i++)
	{
		UE_LOG(LogNetCompression, Log, TEXT("**************************************** ( %d ) %s ****************************************"), i, *TestRange.ToString());
		
		const int32 RandomSeed = FPlatformTime::Cycles();
		FMath::RandInit(RandomSeed);
		
		for (int32 j = 0; j < DataArray1Size; j++)
		{
			const uint8 RandomData = FMath::RandRange(TestRange.Min, TestRange.Max);
			DataArray1[j] = RandomData;
		}
	
		TestHelper.DoTest(HuffmanCode, DataArray1, DataArray1Size);
		TestHelper.DoTest(LZW, DataArray1, DataArray1Size);
		TestHelper.DoTest(LZ4, DataArray1, DataArray1Size);
		TestHelper.DoTest(Zlib, DataArray1, DataArray1Size);
		TestHelper.DoTest(Gzip, DataArray1, DataArray1Size);
		
		UE_LOG(LogNetCompression, Log, TEXT("**************************************** ( %d ) %s ****************************************"), i, *TestRange.ToString());
	}
	
	delete[] DataArray1;
	DataArray1 = nullptr;
}