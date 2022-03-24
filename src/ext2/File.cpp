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
    dataCache(new unsigned char[driver->GetBlockSize()])
{
    auto logBlocksPer = ELoad(driver->superblock.logBlockSize) + 10 - 2;
    for (size_t i = 0; i < cacheLevel; ++i) {
        indexCache[i].Reset(new boot_LE32U[size_t(1) << logBlocksPer]);
    }
    if (driver->LoadINode(&iNode, iNodeNum) != IBlockDevice::NoError) {
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
    if (start + length > GetSize()) {
        return IBlockDevice::AccessOutOfRange;
    }
    auto logBlockSize = ELoad(driver->superblock.logBlockSize) + 10;
    auto blockMask = driver->GetBlockSize() - 1;
    auto startBlock = (start) >> logBlockSize;
    if ((start & blockMask) != 0) {
//        driver->ReadBlock(dataCache.Get(), num);
    }
}

int File::ReadBlock(void* buf, uint32_t blockNum)
{

}

uint32_t File::MapBlock(uint32_t blockNum)
{
    if (blockNum < 12) {
        return ELoad(iNode.blocks[blockNum]);
    }
    blockNum -= 12;
    auto logBlocksPer = ELoad(driver->superblock.logBlockSize) + 10 - 2;
    auto blockCountPer = (size_t(1) << logBlocksPer);
    auto mask = blockCountPer - 1;
    if (blockNum < blockCountPer) {
        driver->ReadBlock(indexCache[0].Get(), ELoad(iNode.blocks[12]));
        return ELoad(indexCache[0][blockNum]);
    }
    blockNum -= blockCountPer;
    blockCountPer <<= logBlocksPer;
    if (blockNum < blockCountPer) {
        driver->ReadBlock(indexCache[1].Get(), ELoad(iNode.blocks[13]));
        driver->ReadBlock(
            indexCache[0].Get(),
            ELoad(indexCache[1][blockNum >> logBlocksPer])
        );
        return ELoad(indexCache[0][blockNum]);
    }
    blockNum -= blockCountPer;
    blockCountPer <<= logBlocksPer;
    if (blockNum < blockCountPer) {
        driver->ReadBlock(indexCache[2].Get(), ELoad(iNode.blocks[14]));
        driver->ReadBlock(
            indexCache[1].Get(),
            ELoad(indexCache[2][blockNum >> (logBlocksPer + logBlocksPer)])
        );
        driver->ReadBlock(
            indexCache[0].Get(),
            ELoad(indexCache[1][(blockNum >> logBlocksPer) & mask])
        );
        return ELoad(indexCache[0][blockNum]);
    }
}

} // namespace ext2
} // namespace boot
