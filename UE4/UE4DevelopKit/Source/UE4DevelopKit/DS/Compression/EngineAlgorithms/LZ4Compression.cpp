#include "LZ4Compression.h"

bool FLZ4Compression::Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	const bool bRet = FCompression::CompressMemory(NAME_LZ4, OutData.Array, OutData.Bytes,
		InData.Array, InData.Bytes);
	
	if (bRet)
	{
		OutData.Bits = OutData.Bytes * 8;
		CompressRatio = OutData.Bytes * 1.0f / InData.Bytes;
	}

	return bRet;
}

bool FLZ4Compression::Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	const bool bRet = FCompression::UncompressMemory(NAME_LZ4, OutData.Array, OutData.Bytes,
		InData.Array, InData.Bytes);

	if (bRet)
	{
		OutData.Bits = OutData.Bytes * 8;
	}

	return bRet;
}
