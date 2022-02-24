#ifndef BOOT_EXT2_EXT2DRIVER_H
#define BOOT_EXT2_EXT2DRIVER_H

#include "boot/IBlockDevice.h"
#include "ext2.h"
#include "boot/UniquePtr.hpp"

namespace boot::ext2 {

class Ext2Driver
{
public:
    Ext2Driver(IBlockDevice* device);
private:
    auto GetBlockSize() const -> size_t;
    void ReinitBuffer(unsigned logBlockSize);
    int  ReadBlock(const unsigned char** buf, uint32_t blockNum);
    static auto Mul2N(uint32_t val, int pwr) -> uint32_t;
    auto GetBlocksCount() const -> uint32_t;

    IBlockDevice* device;
    int logDeviceBlocksPerBlock;
    uint32_t blkMask;
    int logBlkSize;
    boot::UniquePtr<unsigned char[]> buf;
    ::ext2::Superblock superblock;
    uint32_t blocksCount;
};

} // namespace boot::ext2

#endif // BOOT_EXT2_EXT2DRIVER_H
