#include "LzwCompression.h"

#if LZW_USE_CHAR_ARRAY_KEY
FLzwCodeType FLZWEncoder::InitSymbolTable()
{
	SymbolTable.Empty(LZW_SYMBOL_TABLE_LENGTH);
	
	// Init symbol table
	FLzwCodeType NextCode = 0;
	for (; NextCode < LZW_TABLE_INIT_SIZE; NextCode++)
	{
		SymbolTable.Add({ { static_cast<TCHAR>(NextCode) }, NextCode });
	}

	return NextCode;
}

bool FLZWEncoder::Do(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	FLzwCodeType NextCode = InitSymbolTable();
	uint32 CodeLength = LZW_CODE_LENGTH;

	FBitReader BitReader;
	BitReader.SetData(InData.Array, InData.Bits);

	FBitWriter BitWriter(LZW_SYMBOL_TABLE_LENGTH, true);

	FLzwCharType LongestMatchString;

#if LZW_COMPRESSION_DEBUG
	FString CodeDebugStr;
#endif
	
	while (!BitReader.AtEnd())
	{
		uint8 CurrentChar = 0;
		BitReader.SerializeBits(&CurrentChar, LZW_CHAR_LENGTH);

		LongestMatchString.Add(CurrentChar);
		
		// Find the longest match string
		if (!SymbolTable.FindByKey(LongestMatchString))
		{
			LongestMatchString.RemoveAt(LongestMatchString.Num() - 1);
			
			// Output the longest match string's code
			FLzwCodeType& Code = SymbolTable.FindByKey(LongestMatchString)->Code;
			BitWriter.SerializeBits(&Code, CodeLength);

#if LZW_COMPRESSION_DEBUG
			CodeDebugStr.Appendf(TEXT("%d "), Code);
#endif

			// Write the new longest match string to SymbolTable
			LongestMatchString.Add(CurrentChar);
			SymbolTable.Add( { LongestMatchString, NextCode } );
			
			// New match start
			LongestMatchString.Reset();
			LongestMatchString.Add(CurrentChar);
			
			NextCode++;
			FNetCompressionUtils::CountBits(NextCode, CodeLength);
		}
	}

	FLzwCodeType& Code = SymbolTable.FindByKey(LongestMatchString)->Code;
	BitWriter.SerializeBits(&Code, CodeLength);

#if LZW_COMPRESSION_DEBUG
	CodeDebugStr.Appendf(TEXT("%d "), Code);
	UE_LOG(LogNetCompression, Log, TEXT("LZW_Encode Code: %s"), *CodeDebugStr);
#endif

	// Return source data
	OutData.Bytes = BitWriter.GetNumBytes();
	OutData.Bits = BitWriter.GetNumBits();
	FMemory::Memcpy(OutData.Array, BitWriter.GetData(), BitWriter.GetNumBytes());

	CompressRatio = OutData.Bits * 1.0f / InData.Bits;
	
	return true;
}

FLzwCodeType FLzwDecoder::InitSymbolTable()
{
	SymbolTable.Empty(LZW_SYMBOL_TABLE_LENGTH);

	// Init symbol table
	FLzwCodeType NextCode = 0;
	for (; NextCode < LZW_TABLE_INIT_SIZE; NextCode++)
	{
		SymbolTable.Add(NextCode, { static_cast<TCHAR>(NextCode) });
	}

	return NextCode;
}

bool FLzwDecoder::Do(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	uint32 NextCode = InitSymbolTable();
	uint32 CodeLength = LZW_CODE_LENGTH;

	FBitReader BitReader;
	BitReader.SetData(InData.Array, InData.Bits);

	FBitWriter BitWriter(LZW_SYMBOL_TABLE_LENGTH, true);

	// Read the first code
	FLzwCodeType Code = 0;
	BitReader.SerializeBits(&Code, CodeLength);

#if LZW_COMPRESSION_DEBUG
	FString CodeDebugStr;
	CodeDebugStr.Appendf(TEXT("%d "), Code);
#endif	
	
	// Decode the first code
	FLzwCharType OldString = SymbolTable[Code];

	// Output decode result
	for (int32 Index = 0; Index < OldString.Num(); Index++)
	{
		uint8 CharBits = OldString[Index];		
		BitWriter.SerializeBits(&CharBits, LZW_CHAR_LENGTH);
	}
	
	while (!BitReader.AtEnd())
	{
		FNetCompressionUtils::CountBits(NextCode + 1, CodeLength);
		
		// Read the next code
		Code = 0;
		BitReader.SerializeBits(&Code, CodeLength);

#if LZW_COMPRESSION_DEBUG
		CodeDebugStr.Appendf(TEXT("%d "), Code);
#endif		

		FLzwCharType NewString = SymbolTable.FindRef(Code);
		if (NewString.Num())
		{
			OldString.Add(NewString[0]);
			
			// If current code is in SymbolTable, SymbolTable[NextCode] = OldString + NewString[0]
			SymbolTable.Add(NextCode, OldString);
		}
		else
		{
			// If current code isn't in SymbolTable, SymbolTable[Code] = OldString + OldString[0]
			// In this situation, 'Code' must equal to 'NextCode' 
			ensure(Code == NextCode);
			uint8 NewPrefixChar = OldString[0];
			OldString.Add(NewPrefixChar);
			NewString = OldString;
			
			SymbolTable.Add(Code, NewString);
		}
		
		// Output current decode result
		for (int32 Index = 0; Index < NewString.Num(); Index++)
		{
			uint8 CharBits = NewString[Index];
			BitWriter.SerializeBits(&CharBits, LZW_CHAR_LENGTH);
		}

		// Next match start
		OldString = NewString;
		
		NextCode++;
	}

#if LZW_COMPRESSION_DEBUG
	UE_LOG(LogNetCompression, Log, TEXT("LZW_Decode Code: %s"), *CodeDebugStr);
#endif

	// Return source data
	OutData.Bytes = BitWriter.GetNumBytes();
	OutData.Bits = BitWriter.GetNumBits();
	FMemory::Memcpy(OutData.Array, BitWriter.GetData(), BitWriter.GetNumBytes());
	
	return true;
}

bool FLzwCompression::Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{	
	return Encoder.Do(InData, OutData);
}

bool FLzwCompression::Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	return Decoder.Do(InData, OutData);
}

float FLzwCompression::GetRatio() const
{
	return Encoder.GetCompressRatio();
}
#else
FLzwCodeType FLZWEncoder::InitSymbolTable()
{
	SymbolTable.Empty(LZW_SYMBOL_TABLE_LENGTH);
	
	// Init symbol table
	FLzwCodeType NextCode = 0;
	for (; NextCode < LZW_TABLE_INIT_SIZE; NextCode++)
	{
		FString Str;
		Str.AppendChar(NextCode);
		SymbolTable.Add(Str, NextCode);
	}

	return NextCode;
}

bool FLZWEncoder::Do(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	FLzwCodeType NextCode = InitSymbolTable();
	uint32 CodeLength = LZW_CODE_LENGTH;

	FBitReader BitReader;
	BitReader.SetData(InData.Array, InData.Bits);

	FBitWriter BitWriter(LZW_SYMBOL_TABLE_LENGTH, true);

	FLzwCharType LongestMatchString;

#if LZW_COMPRESSION_DEBUG
	FString CodeDebugStr;
#endif
	
	while (!BitReader.AtEnd())
	{
		uint8 CurrentChar = 0;
		BitReader.SerializeBits(&CurrentChar, LZW_CHAR_LENGTH);

		LongestMatchString.AppendChar(CurrentChar);
		
		// Find the longest match string
		if (!SymbolTable.Contains(LongestMatchString))
		{
			LongestMatchString.RemoveAt(LongestMatchString.Len() - 1);
			
			// Output the longest match string's code
			FLzwCodeType* Code = SymbolTable.Find(LongestMatchString);
			BitWriter.SerializeBits(Code, CodeLength);

#if LZW_COMPRESSION_DEBUG
			CodeDebugStr.Appendf(TEXT("%d "), Code);
#endif

			// Write the new longest match string to SymbolTable
			LongestMatchString.AppendChar(CurrentChar);
			SymbolTable.Add(LongestMatchString, NextCode);
			
			// New match start
			LongestMatchString.Reset();
			LongestMatchString.AppendChar(CurrentChar);
			
			NextCode++;
			FNetCompressionUtils::CountBits(NextCode, CodeLength);
		}
	}

	FLzwCodeType* Code = SymbolTable.Find(LongestMatchString);
	BitWriter.SerializeBits(Code, CodeLength);

#if LZW_COMPRESSION_DEBUG
	CodeDebugStr.Appendf(TEXT("%d "), Code);
	UE_LOG(LogNetCompression, Log, TEXT("LZW_Encode Code: %s"), *CodeDebugStr);
#endif

	// Return source data
	OutData.Bytes = BitWriter.GetNumBytes();
	OutData.Bits = BitWriter.GetNumBits();
	FMemory::Memcpy(OutData.Array, BitWriter.GetData(), BitWriter.GetNumBytes());

	CompressRatio = OutData.Bits * 1.0f / InData.Bits;
	
	return true;
}

FLzwCodeType FLzwDecoder::InitSymbolTable()
{
	SymbolTable.Empty(LZW_SYMBOL_TABLE_LENGTH);

	// Init symbol table
	FLzwCodeType NextCode = 0;
	for (; NextCode < LZW_TABLE_INIT_SIZE; NextCode++)
	{
		FString Str;
		Str.AppendChar(static_cast<TCHAR>(NextCode));
		SymbolTable.Add(NextCode, Str);
	}

	return NextCode;
}

bool FLzwDecoder::Do(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	uint32 NextCode = InitSymbolTable();
	uint32 CodeLength = LZW_CODE_LENGTH;

	FBitReader BitReader;
	BitReader.SetData(InData.Array, InData.Bits);

	FBitWriter BitWriter(LZW_SYMBOL_TABLE_LENGTH, true);

	// Read the first code
	FLzwCodeType Code = 0;
	BitReader.SerializeBits(&Code, CodeLength);

#if LZW_COMPRESSION_DEBUG
	FString CodeDebugStr;
	CodeDebugStr.Appendf(TEXT("%d "), Code);
#endif	
	
	// Decode the first code
	FLzwCharType OldString = SymbolTable[Code];

	// Output decode result
	for (int32 Index = 0; Index < OldString.Len(); Index++)
	{
		uint8 CharBits = OldString[Index];		
		BitWriter.SerializeBits(&CharBits, LZW_CHAR_LENGTH);
	}
	
	while (!BitReader.AtEnd())
	{
		FNetCompressionUtils::CountBits(NextCode + 1, CodeLength);
		
		// Read the next code
		Code = 0;
		BitReader.SerializeBits(&Code, CodeLength);

#if LZW_COMPRESSION_DEBUG
		CodeDebugStr.Appendf(TEXT("%d "), Code);
#endif		

		FLzwCharType NewString = SymbolTable.FindRef(Code);
		if (NewString.Len())
		{
			OldString.AppendChar(NewString[0]);
			
			// If current code is in SymbolTable, SymbolTable[NextCode] = OldString + NewString[0]
			SymbolTable.Add(NextCode, OldString);
		}
		else
		{
			// If current code isn't in SymbolTable, SymbolTable[Code] = OldString + OldString[0]
			// In this situation, 'Code' must equal to 'NextCode' 
			ensure(Code == NextCode);
			uint8 NewPrefixChar = OldString[0];
			OldString.AppendChar(NewPrefixChar);
			NewString = OldString;
			
			SymbolTable.Add(Code, NewString);
		}
		
		// Output current decode result
		for (int32 Index = 0; Index < NewString.Len(); Index++)
		{
			uint8 CharBits = NewString[Index];
			BitWriter.SerializeBits(&CharBits, LZW_CHAR_LENGTH);
		}

		// Next match start
		OldString = NewString;
		
		NextCode++;
	}

#if LZW_COMPRESSION_DEBUG
	UE_LOG(LogNetCompression, Log, TEXT("LZW_Decode Code: %s"), *CodeDebugStr);
#endif

	// Return source data
	OutData.Bytes = BitWriter.GetNumBytes();
	OutData.Bits = BitWriter.GetNumBits();
	FMemory::Memcpy(OutData.Array, BitWriter.GetData(), BitWriter.GetNumBytes());
	
	return true;
}

bool FLzwCompression::Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{	
	return Encoder.Do(InData, OutData);
}

bool FLzwCompression::Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	return Decoder.Do(InData, OutData);
}

float FLzwCompression::GetRatio() const
{
	return Encoder.GetCompressRatio();
}
#endif