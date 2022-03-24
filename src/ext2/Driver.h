#ifndef BOOT_EXT2_DRIVER_H
#define BOOT_EXT2_DRIVER_H

#include "boot/IBlockDevice.h"
#include "ext2.h"
#include "boot/UniquePtr.hpp"

namespace boot::ext2 {

class File;

class Driver
{
public:
    friend class File;
    Driver(IBlockDevice* device);
    File OpenINode(uint32_t iNode);
private:
    auto GetBlockSize() const -> size_t;
    void ReinitBuffer(unsigned logBlockSize);
    int  ReadBlock(void* buf, uint32_t blockNum);
    int  ReadBlockSpecial(void* buf, uint32_t blockNum);
    static auto Mul2N(uint32_t val, int pwr) -> uint32_t;
    auto GetBlocksCount() const -> uint32_t;
    auto GetRevisionLevel() const -> uint32_t;
    auto GetINodeSize() const -> size_t;
    auto GetINodesPerGroup() const -> uint32_t;
    int  LoadGroupDesc(::ext2::GroupDesc* desc, uint32_t grpNum);
    int  LoadINode(::ext2::INode* desc, uint32_t iNodeNum);

    IBlockDevice* device;
    int logSectorsPerBlock;
    uint32_t blkMask;
    uint32_t bufStartSect;
    static constexpr auto InvalidSector = uint32_t(-1);
    boot::UniquePtr<unsigned char[]> buf;
    ::ext2::Superblock superblock;
};

} // namespace boot::ext2

#endif // BOOT_EXT2_EXT2DRIVER_H
