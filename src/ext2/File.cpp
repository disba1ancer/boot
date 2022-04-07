#include "File.h"
#include "Driver.h"
#include <exception>
#include <string.h>

namespace boot {
namespace ext2 {

File::File(Driver* driver, uint32_t iNodeNum) :
    driver(driver),
    iNode({}),
    lastBlock(0),
    logBlocksPer(driver->LogBlockSize() - 2),
    blockCountPer1(size_t(1) << logBlocksPer),
    blockCountPer2(blockCountPer1 << logBlocksPer),
    mask1(~(blockCountPer1 - 1)),
    mask2(~(blockCountPer2 - 1)),
    lastDataBlock(0),
    dataCache(new unsigned char[driver->GetBlockSize()])
{
    auto logBlocksPer = ELoad(driver->superblock.logBlockSize) + 10 - 2;
    for (size_t i = 0; i < cacheLevel; ++i) {
        indexCache[i].Reset(new boot_LE32U[size_t(1) << logBlocksPer]);
    }
    if (driver->LoadINode(&iNode, iNodeNum) != IBlockDevice::NoError) {
        std::terminate();
    }
    if (GetSize() > (uint64_t(12 + blockCountPer1 + blockCountPer2) << driver->LogBlockSize())) {
        auto result = driver->ReadBlock(indexCache[2].Get(), ELoad(iNode.blocks[14]));
        if (result != IBlockDevice::NoError) {
            std::terminate();
        }
    }
    if (ReadBlock(dataCache.Get(), lastDataBlock) != IBlockDevice::NoError) {
        std::terminate();
    }
}

uint32_t File::GetMode() const
{
    return (uint32_t(ELoad(iNode.modeHigh)) << 16) + ELoad(iNode.mode);
}

uint32_t File::GetUID() const
{
    return (uint32_t(ELoad(iNode.uidHigh)) << 16) + ELoad(iNode.uid);
}

uint64_t File::GetSize() const
{
    uint64_t high = IsDirectory() * ELoad(iNode.dirAcl);
    return (high << 32) + ELoad(iNode.sizeLow);
}

uint32_t File::GetGID() const
{
    return (uint32_t(ELoad(iNode.gidHigh)) << 16) + ELoad(iNode.gid);
}

bool File::IsDirectory() const
{
    return (GetMode() & ::ext2::Mode_TypeMask) == ::ext2::Mode_Dir;
}

int File::Read(void* buf, uint64_t start, size_t length)
{
    if (start + length < start || start + length > GetSize()) {
        return IBlockDevice::AccessOutOfRange;
    }
    auto logBlockSize = driver->LogBlockSize();
    auto blockSize = driver->GetBlockSize();
    auto blockMask = blockSize - 1;
    auto bufferOffset = size_t(start & blockMask);
    auto currentBlock = uint32_t(start >> logBlockSize);

    while (length > 0) {
        auto min = boot::Min(length, blockSize - bufferOffset);
        if (bufferOffset == 0 && length >= blockSize) {
            auto result = ReadBlock(buf, currentBlock);
            if (result != IBlockDevice::NoError) {
                return result;
            }
        } else {
            if (lastDataBlock != currentBlock) {
                auto result = ReadBlock(dataCache.Get(), currentBlock);
                if (result != IBlockDevice::NoError) {
                    lastDataBlock = 0;
                    return result;
                }
                lastDataBlock = currentBlock;
            }
            memcpy(buf, dataCache.Get() + bufferOffset, min);
            bufferOffset = 0;
        }
        ++currentBlock;
        length -= min;
        buf = (unsigned char*)buf + min;
    }
    return IBlockDevice::NoError;
}

Driver& File::GetDriver() const
{
    return *driver;
}

int File::ReadBlock(void* buf, uint32_t blockNum)
{
    if (blockNum >= (ELoad(iNode.sectors) >> (driver->LogBlockSize() - 9))) {
        return IBlockDevice::AccessOutOfRange;
    }
    return driver->ReadBlock(buf, MapBlock(blockNum));
}

uint32_t File::MapBlock(uint32_t blockNum)
{
    if (blockNum < 12) {
        return ELoad(iNode.blocks[blockNum]);
    }
    auto sb = 12 + blockCountPer1 + blockCountPer2;
    auto mask = blockCountPer1 - 1;

    auto bn = blockNum - sb;
    auto lb = lastBlock - sb;

    if ((bn & mask2) != (lb & mask2)) {
        if (bn < -sb) {
            driver->ReadBlock(
                indexCache[1].Get(),
                ELoad(indexCache[2][(bn >> (logBlocksPer + logBlocksPer)) & mask])
            );
        } else {
            driver->ReadBlock(indexCache[1].Get(), ELoad(iNode.blocks[13]));
        }
    }

    if ((bn & mask1) != (lb & mask1)) {
        if (bn < -sb || bn >= -(blockCountPer2)) {
            driver->ReadBlock(
                indexCache[0].Get(),
                ELoad(indexCache[1][(bn >> (logBlocksPer)) & mask])
            );
        } else {
            driver->ReadBlock(indexCache[0].Get(), ELoad(iNode.blocks[12]));
        }
    }

    lastBlock = blockNum;
    return ELoad(indexCache[0][bn & mask]);
}

} // namespace ext2
} // namespace boot
