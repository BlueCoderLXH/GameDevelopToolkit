#pragma once
#include "../NetCompressionInterface.h"

#define HUFFMAN_NODE_NULL_INDEX -1

#define HUFFMAN_COMPRESSION_DEBUG 0

struct FHuffmanConstants
{
	static constexpr uint16 TreeNodeMaxCount = 256;
	
	static constexpr uint16 DataMaxBits = 8;
	static constexpr uint16 CodeMaxBits = 8;
	
	static constexpr uint16 CodeSetMaxBits = 16;
};

/*
 * Huffman tree node
 */
struct FHuffmanNode
{
	// Leaf node data ( Only for leaf node )
	FNetCompressionDataType Data = 0;

	// Node frequency
	uint32 Frequency = 0;

	// Left child node index in huffman tree array
	int32 LeftIndex = HUFFMAN_NODE_NULL_INDEX;
	// Right child node index in huffman tree array
	int32 RightIndex = HUFFMAN_NODE_NULL_INDEX;

	// Node height: from this to leaf node
	uint32 Height = 0;

	bool IsLeaf() const { return LeftIndex == HUFFMAN_NODE_NULL_INDEX && RightIndex == HUFFMAN_NODE_NULL_INDEX; }

	bool operator<(const FHuffmanNode& Other) const
	{
		return Frequency < Other.Frequency;
	}
};

/*
 * Huffman tree
 */
struct FHuffmanTree
{
private:
	// Huffman tree array struct
	// For huffman tree features, array struct is the best way to save huffman tree infos
	TArray<FHuffmanNode> Tree;

	// Huffman node frequency count 
	TArray<uint16> FrequencyCount;

	// Huffman tree depth from max(root to leaf)
	uint32 Depth = 0;

public:
	const TArray<FHuffmanNode>& Get() const { return Tree; }

	uint32 GetNum() const { return Tree.Num(); }
	uint32 GetDepth() const { return Depth; }

	/**
	 * @brief Build huffman tree for input data 
	 * @param InData Input data wrapper struct
	 * @return Success or not
	 */
	bool Build(const FNetCompressionDataWrap& InData);

#if HUFFMAN_COMPRESSION_DEBUG
	/**
	 * @brief Print huffman tree(level order) with tree format
	 */
	void Print() const;
#endif
};

/*
 * Huffman code
 */
struct FHuffmanCode
{
public:
	// For encoding
	TMap<FNetCompressionDataType, FNetCompressionCodeType> DataToCode;
	uint32 EncodeLength = 0;
	uint16 EncodeSetLength = 0;;

	// For decoding
	TMap<FNetCompressionCodeType, FNetCompressionDataType> CodeToData;
	uint32 DecodeLength = 0;
	uint16 DecodeSetLength = 0;

	// Compression ratio
	float CompressRatio = 0.f;

private:
	// const int32 MaxBits = MAX_PACKET_SIZE * 16;
	const int32 MaxBits = 262144; // 2^18
	FBitWriter BitWriter = FBitWriter(MaxBits);
	FBitReader BitReader = FBitReader(nullptr, MaxBits);

public:
	/*
	 * Encode
	 * From source input data to huffman code 
	 */
	bool Encode(const FHuffmanTree& HuffmanTree, const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData);

	/*
	 * Decode
	 * From huffman code to source input data 
	 */
	bool Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData);

#if HUFFMAN_COMPRESSION_DEBUG
	void PrintEncodeDebugInfo();
	
	void PrintDecodeDebugInfo();
#endif

private:
	/*
	 * Build huffman encode map structs
	 */
	bool BuildEncodeSet(const FHuffmanTree& HuffmanTree);

	/*
	 * Build huffman code for input data
	 */
	bool BuildEncode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData);
};

/*
 * Huffman compression algorithm
 */
struct FHuffmanCompression : public FNetCompressionInterface
{
private:
	FHuffmanTree HuffmanTree;
	FHuffmanCode HuffmanCode;

public:
	/*
	 * Encode
	 * From source input data to huffman code 
	 */
	virtual bool Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) override;

	/*
	 * Decode
	 * From huffman code to source input data 
	 */
	virtual bool Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData) override;

	/*
	 * Get the compression ratio
	 */
	virtual float GetRatio() const override
	{
		return HuffmanCode.CompressRatio;
	}

	virtual FString GetName() const override { return TEXT("Huffman"); }
};
