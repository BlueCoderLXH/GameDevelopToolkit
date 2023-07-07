#include "GzipCompression.h"

bool FGzipCompression::Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	const bool bRet = FCompression::CompressMemory(NAME_Gzip, OutData.Array, OutData.Bytes,
	InData.Array, InData.Bytes);

	if (bRet)
	{
		OutData.Bits = OutData.Bytes * 8;
		CompressRatio = OutData.Bytes * 1.0f / InData.Bytes;
	}

	return bRet;
}

bool FGzipCompression::Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	const bool bRet = FCompression::UncompressMemory(NAME_Gzip, OutData.Array, OutData.Bytes,
		InData.Array, InData.Bytes);

	if (bRet)
	{
		OutData.Bits = OutData.Bytes * 8;
	}

	return bRet;
}
