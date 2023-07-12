# Simple Network Compression Framework
## 一、包括的算法
- **Huffman Code**
- **LZW**
- **LZ4**
- **Zlib**
- **Gzip**

## 二、性能测试
### 2.1 BP_CompressionTest蓝图
- 将蓝图 Blueprint'/Game/Blueprints/BP_CompressionTest.BP_CompressionTest' 放进map中
### 2.2 配置测试数据
- **DataArrayCount**\
  测试数据量（单位：字节），这里设置1024的原因是UE底层每个Packet的MTU即为1024，尽量贴合UE测试环境
- **TestTimes**\
  测试次数，测试1000次，最后求得性能数据的平均值。
- **TestRange**\
  测试数据范围，类型为uint8，单个字节，最大也就是256。

> 具体参见：\
[《UE4 Notes》网络数据优化（上）常用压缩算法对比](https://zhuanlan.zhihu.com/p/642628757)