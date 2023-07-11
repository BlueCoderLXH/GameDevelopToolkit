#pragma once
#include "../NetCompressionInterface.h"

struct FLZ4Compression : public FNetCompressionInterface
{
	virtual bool Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) override;
	virtual bool Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) override;

	virtual FString GetName() const override { return TEXT("LZ4    "); }
};
