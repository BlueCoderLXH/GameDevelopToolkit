#include "HuffmanCompression.h"

bool FHuffmanTree::Build(const FNetCompressionDataWrap& InData)
{
	FrequencyCount.Init(0, FHuffmanConstants::TreeNodeMaxCount);

	FBitReader BitReader(InData.Array, InData.Bits);

	// Count the frequency of data item
	while (!BitReader.AtEnd())
	{
		FNetCompressionCodeType Data = 0;
		BitReader.SerializeBits(&Data, FHuffmanConstants::DataMaxBits);
		FrequencyCount[Data]++;
	}

	TArray<FHuffmanNode> PriorityQueue;
	// Heap sort the huffman leaf nodes
	for (int32 i = 0; i < FrequencyCount.Num(); i++)
	{
		if (FrequencyCount[i])
		{
			PriorityQueue.HeapPush({ static_cast<FNetCompressionDataType>(i), FrequencyCount[i] });
		}
	}

	// Huffman tree feature: NodeNum = LeafNodeNum * 2 - 1
	const uint32 HuffmanTreeNodeCount = PriorityQueue.Num() * 2 - 1;
	Tree.Init({}, HuffmanTreeNodeCount);

	uint32 InsertIndex = HuffmanTreeNodeCount;
	// Build huffman tree
	while(PriorityQueue.Num() > 1)
	{
		const uint32 LeftNodeIndex = (--InsertIndex);
		FHuffmanNode& LeftNode = Tree[LeftNodeIndex];
		PriorityQueue.HeapPop(LeftNode);

		const uint32 RightNodeIndex = (--InsertIndex);
		FHuffmanNode& RightNode = Tree[RightNodeIndex];
		PriorityQueue.HeapPop(RightNode);

		FHuffmanNode ParentNode;
		ParentNode.Frequency = LeftNode.Frequency + RightNode.Frequency;
		ParentNode.LeftIndex = LeftNodeIndex;
		ParentNode.RightIndex = RightNodeIndex;
		ParentNode.Height = FMath::Max(LeftNode.Height, RightNode.Height) + 1;

		PriorityQueue.HeapPush(ParentNode);
	}

	// Set Root
	constexpr uint32 RootIndex = 0;
	PriorityQueue.HeapPop(Tree[RootIndex]);

	// For the root node: height == depth
	Depth = FMath::Max(Tree[RootIndex].Height, 1u);

#if HUFFMAN_COMPRESSION_DEBUG
	Print();
#endif	

	return true;
}

#if HUFFMAN_COMPRESSION_DEBUG
void FHuffmanTree::Print() const
{
	static const FString BlankStr = TEXT("       ");
	static const FString EmptyNodeStr = TEXT("_______");
	
	TQueue<FHuffmanNode> NodeQueue;
	TArray<FHuffmanNode> LevelNodes;
	
	NodeQueue.Enqueue(Tree[0]);

	FHuffmanNode CurrentNode;
	uint8 NodeDepth = Depth;

	UE_LOG(LogNetCompression, Log, TEXT("Huffman Tree(Frequency/Data):"));
	// Level order
	while(!NodeQueue.IsEmpty())
	{
		// Get all current level nodes
		while(NodeQueue.Dequeue(CurrentNode))
		{
			LevelNodes.Add(CurrentNode);
		}

		// Enqueue nodes by level order(from left to right)
		for (int32 Index = 0; Index < LevelNodes.Num(); Index++)
		{
			const FHuffmanNode& LevelNode = LevelNodes[Index];
			FHuffmanNode NullNode;
			
			// Null Node
			if (!LevelNode.Frequency)
			{
				// Make it as a FBT(Full-Binary-Tree)
				if (NodeDepth)
				{
					NodeQueue.Enqueue(NullNode);
					NodeQueue.Enqueue(NullNode);
				}
			}
			// Actual node
			else
			{
				if (LevelNode.LeftIndex > 0)
				{
					NodeQueue.Enqueue(Tree[LevelNode.LeftIndex]);
				}
				// Make it as a FBT(Full-Binary-Tree)
				else if (NodeDepth)
				{
					NodeQueue.Enqueue(NullNode);
				}

				if (LevelNode.RightIndex > 0)
				{
					NodeQueue.Enqueue(Tree[LevelNode.RightIndex]);
				}
				// Make it as a FBT(Full-Binary-Tree)
				else if (NodeDepth)
				{
					NodeQueue.Enqueue(NullNode);	
				}
			}
		}

		const uint32 NodeGapCount = FMath::Pow(2, NodeDepth) - 1;
		FString LevelNodeStr;
		// Print nodes by level order(from left to right)
		for (int32 Index = 0; Index < LevelNodes.Num(); Index++)
		{
			const FHuffmanNode& LevelNode = LevelNodes[Index];
			
			// Null Node
			if (!LevelNode.Frequency)
			{
				for (uint32 i = 0; i < NodeGapCount; i++)
				{
					LevelNodeStr.Append(BlankStr);
				}
				LevelNodeStr.Append(EmptyNodeStr);
				for (uint32 i = 0; i < NodeGapCount + 1; i++)
				{
					LevelNodeStr.Append(BlankStr);
				}
			}
			// Actual node
			else
			{
				for (uint32 i = 0; i < NodeGapCount; i++)
				{
					LevelNodeStr.Append(BlankStr);
				}
				LevelNodeStr.Append(FString::Printf(TEXT("%3d/%-3d"), LevelNode.Frequency, LevelNode.IsLeaf() ? LevelNode.Data : -1));
				for (uint32 i = 0; i < NodeGapCount + 1; i++)
				{
					LevelNodeStr.Append(BlankStr);
				}
			}
		}		
		UE_LOG(LogNetCompression, Log, TEXT("%s"), *LevelNodeStr);

		LevelNodes.Reset();
		NodeDepth--;
	}
}
#endif

bool FHuffmanCode::BuildEncodeSet(const FHuffmanTree& HuffmanTree)
{
	if (HuffmanTree.GetNum() <= 0)
	{
		return false;
	}

	struct FHuffmanCodeTraverseNode
	{
		FHuffmanNode TreeNode;
		FNetCompressionCodeType HuffmanCode = 0;
		uint8 Depth = 0;
	};

	DataToCode.Reset();

	// Temp stack to store traverse nodes for saving memory
	TArray<FHuffmanCodeTraverseNode> NodeStack;
	NodeStack.Reserve(HuffmanTree.GetNum());

	// Push root
	NodeStack.Push({ HuffmanTree.Get()[0], 0, 0 });

	const uint8 TreeDepth = HuffmanTree.GetDepth();
	
	// Traverse huffman tree to make huffman code
	while(NodeStack.Num() > 0)
	{
		const FHuffmanCodeTraverseNode& CurrentTraverseNode = NodeStack.Pop();

		const FHuffmanNode& CurrentNode = CurrentTraverseNode.TreeNode;
		const FNetCompressionCodeType& CurrentHuffmanCode = CurrentTraverseNode.HuffmanCode;
		const uint8 CurrentDepth = CurrentTraverseNode.Depth;

		// Make code for leaf node
		if (CurrentNode.IsLeaf())
		{
			const FNetCompressionCodeType FinalCode = CurrentHuffmanCode << ( TreeDepth - CurrentDepth );
			
			// Calc final huffman code: align the rest bits behind 
			DataToCode.Add(CurrentNode.Data, FinalCode);
		}
		else
		{
			const uint8 NextDepth = CurrentDepth + 1u;
			
			// Traverse right
			if (CurrentNode.RightIndex > 0)
			{
				const FNetCompressionCodeType RightChildHuffmanCode = (CurrentHuffmanCode << 1) + 1;
				NodeStack.Push({ HuffmanTree.Get()[CurrentNode.RightIndex], RightChildHuffmanCode, NextDepth });
			}
			
			// Traverse left
			if (CurrentNode.LeftIndex > 0)
			{
				const FNetCompressionCodeType LeftChildHuffmanCode = CurrentHuffmanCode << 1;
				NodeStack.Push({ HuffmanTree.Get()[CurrentNode.LeftIndex], LeftChildHuffmanCode, NextDepth });
			}
		}
	}
	
	EncodeLength = TreeDepth;
	
	return true;
}

bool FHuffmanCode::BuildEncode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	BitWriter.Reset();

	// Write code length
	BitWriter.SerializeBits(&EncodeLength, FHuffmanConstants::CodeMaxBits);
	
	// Write huffman code set length
	EncodeSetLength = DataToCode.Num() * (FHuffmanConstants::DataMaxBits + EncodeLength);
	BitWriter.SerializeBits(&EncodeSetLength, FHuffmanConstants::CodeSetMaxBits);
	
	// Write huffman code set
	for (const TTuple<FNetCompressionDataType, FNetCompressionCodeType>& Pair : DataToCode)
	{
		FNetCompressionCodeType Code = Pair.Value;
		FNetCompressionDataType Data = Pair.Key;
		
		BitWriter.SerializeBits(&Code, EncodeLength);
		BitWriter.SerializeBits(&Data, FHuffmanConstants::DataMaxBits);
	}

	BitReader.SetData(InData.Array, InData.Bits);
	// Write huffman encode
	while (!BitReader.AtEnd())
	{
		FNetCompressionDataType Data = 0;
		BitReader.SerializeBits(&Data, FHuffmanConstants::DataMaxBits);

		FNetCompressionCodeType Code = DataToCode[Data];

		// TODO: try dynamic=length bits
		// Write code with fixed-length bits
		BitWriter.SerializeBits(&Code, EncodeLength);
	}

	if (BitWriter.IsError())
	{
		return false;
	}

	// Return the output data
	OutData.Bytes = BitWriter.GetNumBytes();
	OutData.Bits = BitWriter.GetNumBits();
	FMemory::Memcpy(OutData.Array, BitWriter.GetData(), BitWriter.GetNumBytes());

	CompressRatio = OutData.Bits * 1.0f / InData.Bits;

#if HUFFMAN_COMPRESSION_DEBUG
	PrintEncodeDebugInfo();
#endif
	
	return true;
}

#if HUFFMAN_COMPRESSION_DEBUG
void FHuffmanCode::PrintEncodeDebugInfo()
{
	UE_LOG(LogNetCompression, Log, TEXT("Encode CodeLength: \t\t%d"), EncodeLength);
	
	UE_LOG(LogNetCompression, Log, TEXT("Encode CodeSetLength: \t%d"), EncodeSetLength);

	FString DataCodeStr;
	for (const TTuple<FNetCompressionDataType, FNetCompressionCodeType> Item : DataToCode)
	{
		FString CodeStr = FNetCompressionUtils::ToBinaryString(Item.Value, EncodeLength);
		FString DataStr = FNetCompressionUtils::ToBinaryString(Item.Key, FHuffmanConstants::DataMaxBits);
		DataCodeStr.Append(FString::Printf(TEXT("%d(%s):%d(%s) "), Item.Key, *DataStr, Item.Value, *CodeStr));
	}
	UE_LOG(LogNetCompression, Log, TEXT("BuildCode DataToCode: \t%s"), *DataCodeStr);
}
#endif

bool FHuffmanCode::Encode(const FHuffmanTree& HuffmanTree, const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	if (!BuildEncodeSet(HuffmanTree))
	{
		return false;
	}

	return BuildEncode(InData, OutData);
}

#if HUFFMAN_COMPRESSION_DEBUG
void FHuffmanCode::PrintDecodeDebugInfo()
{
	UE_LOG(LogNetCompression, Log, TEXT("Decode CodeLength: \t\t%d"), DecodeLength);

	UE_LOG(LogNetCompression, Log, TEXT("Decode CodeSetLength: \t%d"), DecodeSetLength);

	FString CodeDataStr;
	for (const TTuple<FNetCompressionDataType, FNetCompressionCodeType> Item : CodeToData)
	{
		FString CodeStr = FNetCompressionUtils::ToBinaryString(Item.Key, DecodeLength);
		FString DataStr = FNetCompressionUtils::ToBinaryString(Item.Value, FHuffmanConstants::DataMaxBits);
		CodeDataStr.Append(FString::Printf(TEXT("%d(%s):%d(%s) "), Item.Key, *CodeStr, Item.Value, *DataStr));
	}
	UE_LOG(LogNetCompression, Log, TEXT("Decode CodeToData: \t\t%s"), *CodeDataStr);
}
#endif

bool FHuffmanCode::Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	BitReader.SetData(InData.Array, InData.Bits);
	if (BitReader.IsError())
	{
		return false;
	}

	// Read huffman code length
	BitReader.SerializeBits(&DecodeLength, FHuffmanConstants::CodeMaxBits);

	// Read huffman code set length
	BitReader.SerializeBits(&DecodeSetLength, FHuffmanConstants::CodeSetMaxBits);
	const uint32 HuffmanCodeSetCount = DecodeSetLength / (FHuffmanConstants::DataMaxBits + DecodeLength);

	CodeToData.Reset();
	// Recover huffman code set
	for (uint32 i = 0; i < HuffmanCodeSetCount; i++)
	{
		FNetCompressionCodeType Code = 0;
		BitReader.SerializeBits(&Code, DecodeLength);
		FNetCompressionDataType Data;
		BitReader.SerializeBits(&Data, FHuffmanConstants::DataMaxBits);
		
		CodeToData.Add(Code, Data);
	}

#if HUFFMAN_COMPRESSION_DEBUG	
	PrintDecodeDebugInfo();
#endif
	
	BitWriter.Reset();
	
	// Decode
	while (!BitReader.AtEnd())
	{
		// Read fixed-length code
		FNetCompressionCodeType HuffmanCode = 0;
		BitReader.SerializeBits(&HuffmanCode, DecodeLength);
		
		FNetCompressionDataType* Data = CodeToData.Find(HuffmanCode);
		if (Data)
		{
			BitWriter.SerializeBits(Data, FHuffmanConstants::DataMaxBits);
		}
		else
		{
			UE_LOG(LogNetCompression, Error, TEXT("Decode() failed to decode for %d"), HuffmanCode);
		}
	}
	
	if (BitWriter.IsError())
	{
		return false;
	}

	// Return source data
	OutData.Bytes = BitWriter.GetNumBytes();
	OutData.Bits = BitWriter.GetNumBits();
	FMemory::Memcpy(OutData.Array, BitWriter.GetData(), BitWriter.GetNumBytes());
	
	return true;
}

bool FHuffmanCompression::Encode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	if (!HuffmanTree.Build(InData))
	{
		return false;
	}

	return HuffmanCode.Encode(HuffmanTree, InData, OutData);
}

bool FHuffmanCompression::Decode(const FNetCompressionDataWrap& InData, FNetCompressionDataWrap& OutData)
{
	return HuffmanCode.Decode(InData, OutData);
}
