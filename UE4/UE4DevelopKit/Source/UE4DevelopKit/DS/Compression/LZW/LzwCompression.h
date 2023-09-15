#pragma once
#include "../NetCompressionInterface.h"

#define LZW_COMPRESSION_DEBUG	0

/*
	LZW_USE_CHAR_ARRAY_KEY      FLzwCharType
	0                           FString
	1                           TArray<TCHAR>

	FLzwCharType use FString vs TArray<TCHAR>
	- FString is faster than TArray<TCHAR>
	- FString can't deal with 0
*/
#define LZW_USE_CHAR_ARRAY_KEY 0

#if LZW_USE_CHAR_ARRAY_KEY
/*
	FLzwCharType use TArray<TCHAR> instead of FString:
	FString will make 0 to be the string end char, so FString will ignore 0 if add it
*/
typedef TArray<TCHAR> FLzwCharType;
typedef uint32  FLzwCodeType;


#define LZW_TABLE_INIT_SIZE		256u
#define LZW_CHAR_LENGTH			8U
#define LZW_CODE_LENGTH			9u
#define LZW_SYMBOL_TABLE_LENGTH	262144u // 2^18

/*
 * LZW compression encoder
 */
struct FLZWEncoder
{
private:
	struct FLzwSymbolItem
	{
		FLzwCharType Key;
		FLzwCodeType Code;

		// Override operator== to support TArray::FindByKey
		bool operator==(const FLzwCharType& InKey) const
		{
			if (Key.Num() != InKey.Num()) return false;

			for (int32 Index = 0; Index < Key.Num(); Index++)
			{
				if (Key[Index] != InKey[Index]) return false;
			}

			return true;
		}
	};
	
	TArray<FLzwSymbolItem> SymbolTable;
	float CompressRatio = 0.f;
	
public:
	float GetCompressRatio() const { return CompressRatio; }
	
	bool Do(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData);

private:
	FLzwCodeType InitSymbolTable();
};

/*
 * LZW compression decoder
 */
struct FLzwDecoder
{
private:
	TMap<FLzwCodeType, FLzwCharType> SymbolTable;
	
public:
	bool Do(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData);

private:
	FLzwCodeType InitSymbolTable();
};

/*
 * LZW compression algorithm
 */
struct FLzwCompression : public FNetCompressionInterface
{
private:
	FLZWEncoder Encoder;
	FLzwDecoder Decoder;

public:
	/*
	 * Encode
	 * From source input data to encode 
	 */
	virtual bool Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) override;

	/*
	 * Decode
	 * From encode to decode 
	 */
	virtual bool Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) override;

	/*
	 * Get the compression ratio
	 */
	virtual float GetRatio() const override;

	virtual FString GetName() const override { return TEXT("LZW    "); }	
};
#else
/*
	FLzwCharType use FString vs TArray<TCHAR>
	- FString is faster than TArray<TCHAR>
	- FString can't deal with 0
*/
typedef FString FLzwCharType;
typedef uint32  FLzwCodeType;


#define LZW_TABLE_INIT_SIZE		256u
#define LZW_CHAR_LENGTH			8U
#define LZW_CODE_LENGTH			9u
#define LZW_SYMBOL_TABLE_LENGTH	262144u // 2^18

/*
 * LZW compression encoder
 */
struct FLZWEncoder
{
private:
	// FString is case-insensitive by default, so we should make it case-sensitive
	struct CaseSensitive_KeyFuncs : BaseKeyFuncs<TPair<FLzwCharType, FLzwCodeType>, FLzwCharType, false>
	{
		static FORCEINLINE const FLzwCharType& GetSetKey(const TPair<FLzwCharType, FLzwCodeType>& Element) { return Element.Key; }
		static FORCEINLINE FLzwCodeType  GetKeyHash(const FLzwCharType& Key) { return GetTypeHash(Key); }
		static FORCEINLINE bool Matches(const FLzwCharType& A, const FLzwCharType& B) { return A.Equals(B, ESearchCase::CaseSensitive); }
	};	
	
	TMap<FLzwCharType, FLzwCodeType, FDefaultSetAllocator, CaseSensitive_KeyFuncs> SymbolTable;
	float CompressRatio = 0.f;
	
public:
	float GetCompressRatio() const { return CompressRatio; }
	
	bool Do(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData);

private:
	FLzwCodeType InitSymbolTable();
};

/*
 * LZW compression decoder
 */
struct FLzwDecoder
{
private:
	TMap<FLzwCodeType, FLzwCharType> SymbolTable;
	
public:
	bool Do(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData);

private:
	FLzwCodeType InitSymbolTable();
};

/*
 * LZW compression algorithm
 */
struct FLzwCompression : public FNetCompressionInterface
{
private:
	FLZWEncoder Encoder;
	FLzwDecoder Decoder;

public:
	/*
	 * Encode
	 * From source input data to encode 
	 */
	virtual bool Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) override;

	/*
	 * Decode
	 * From encode to decode 
	 */
	virtual bool Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) override;

	/*
	 * Get the compression ratio
	 */
	virtual float GetRatio() const override;

	virtual FString GetName() const override { return TEXT("LZW"); }
};
#endif