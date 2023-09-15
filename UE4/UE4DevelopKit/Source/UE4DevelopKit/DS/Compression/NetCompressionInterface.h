#pragma once
#include <CoreMinimal.h>

typedef uint16 FNetCompressionCodeType;
typedef uint8 FNetCompressionDataType;

DECLARE_LOG_CATEGORY_EXTERN(LogNetCompression, Log, All);

/*
 * Net compression util functions
 */
struct FNetCompressionUtils
{
	static FString ToBinaryString(const FNetCompressionDataType* DataArray, const int32 CountBits);
	
	static FString ToBinaryString(const uint64 Data, const uint32 SizeBits);

	static void CountBits(const uint64 Data, uint32& CachedBits) { while (Data >> CachedBits) CachedBits++; }
};

/*
 * Network compression data wrap struct ( Input/Output )
 */
struct FNetCompressionDataWrap
{
	FNetCompressionDataType* Array = nullptr;
	int32 Bits = 0;
	int32 Bytes = 0;

	FString ToString() const
	{
		return FNetCompressionUtils::ToBinaryString(Array, Bits);
	}
};

/*
 * Network compression enum type
 */
enum ENetworkCompressionType : uint8
{
	Min				= 0,
	
	HuffmanCode		= 0,
	LZW				= 1,
	LZ4				= 2,
	Zlib			= 3,
	Gzip			= 4,

	Max
};

/*
 * CompressionInterface
 */
struct FNetCompressionInterface
{
protected:
	float CompressRatio = 0.f;

public:
	virtual ~FNetCompressionInterface() {} 
	
	/*
	 * Encode
	 * From source input data to encode 
	 */
	virtual bool Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) = 0;

	/*
	 * Decode
	 * From encode to decode 
	 */
	virtual bool Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) = 0;

	/*
	 * Get the compression ratio
	 */
	virtual float GetRatio() const { return CompressRatio; }

	virtual FString GetName() const = 0;
};

/*
 * Net compression factory
 */
struct FNetCompressionFactory
{
	static TSharedPtr<FNetCompressionInterface> New(const ENetworkCompressionType& CompressionType);
};