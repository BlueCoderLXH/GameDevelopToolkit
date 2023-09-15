#include "NetCompressionInterface.h"

#include "Huffman/HuffmanCompression.h"
#include "LZW/LzwCompression.h"
#include "EngineAlgorithms/LZ4Compression.h"
#include "EngineAlgorithms/ZlibCompression.h"
#include "EngineAlgorithms/GzipCompression.h"

DEFINE_LOG_CATEGORY(LogNetCompression);

FString FNetCompressionUtils::ToBinaryString(const FNetCompressionDataType* DataArray, const int32 CountBits)
{
	const  int32 CountBytes = (CountBits + 7) >> 3;
		
	FString Ret;
	int32 ConvertBits = 0;
	for (int32 i = 0; i < CountBytes; i++)
	{
		const FNetCompressionDataType Data = *(DataArray + i);

		int32 Factor = 128;
		while (Factor && ConvertBits < CountBits)
		{
			Ret.AppendInt((Data / Factor) % 2);
			Factor >>= 1;
			ConvertBits++;
		}

		Ret.AppendChar(' ');
	}

	return Ret;		
}

FString FNetCompressionUtils::ToBinaryString(const uint64 Data, const uint32 SizeBits)
{
	if (SizeBits <= 0)
	{
		return TEXT("");
	}
		
	FString Ret;
	uint64 Factor = FMath::Pow(2, SizeBits - 1);
	while (Factor)
	{
		Ret.AppendInt((Data / Factor) % 2);
		Factor >>= 1;
	}
	return Ret;		
}

TSharedPtr<FNetCompressionInterface> FNetCompressionFactory::New(const ENetworkCompressionType& CompressionType)
{
	TSharedPtr<FNetCompressionInterface> CompressionHandler = nullptr;
	switch (CompressionType)
	{
	case HuffmanCode:
		CompressionHandler = MakeShared<FHuffmanCompression>();
		break;
	case LZW:
		CompressionHandler = MakeShared<FLzwCompression>();
		break;		
	case LZ4:
		CompressionHandler = MakeShared<FLZ4Compression>();
		break;
	case Zlib:
		CompressionHandler = MakeShared<FZlibCompression>();
		break;
	case Gzip:
		CompressionHandler = MakeShared<FGzipCompression>();
		break;
	}
	check(CompressionHandler.IsValid());
	return CompressionHandler;
}
