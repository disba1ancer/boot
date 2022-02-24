#include "Ext2Driver.h"
#include <exception>
#include "boot/util.h"
#include "string.h"

namespace boot::ext2 {

Ext2Driver::Ext2Driver(IBlockDevice* device) :
    device(device),
    blocksCount(0)
{
    ReinitBuffer(0);
    const unsigned char* superblBytes;
    if (ReadBlock(&superblBytes, 1) != IBlockDevice::NoError) {
        std::terminate();
    }
    memcpy(&superblock, superblBytes, sizeof(superblock));
    ReinitBuffer(boot_LE32ULoad(&superblock.logBlockSize));
    blocksCount = boot_LE32ULoad(&superblock.blocksCount);// TODO: Add store functions
}

void Ext2Driver::ReinitBuffer(unsigned logBlockSize)
{
    auto devBlockSize = device->GetBlockSize();
    auto logDevBlkSz = boot_Log2U64(devBlockSize);
    if ((1 << logDevBlkSz) != devBlockSize) {
        std::terminate();
    }
    logDeviceBlocksPerBlock = 10 + int(logBlockSize) - logDevBlkSz;
    blkMask = 0;
    if (logDeviceBlocksPerBlock < 0) {
        blkMask = (uint32_t(1) << uint32_t(-logDeviceBlocksPerBlock)) - 1;
    }
    logBlkSize = 10 + int(logBlockSize);
    buf.Reset();
    buf.Reset(new unsigned char[Mul2N(devBlockSize, Max(0, logDeviceBlocksPerBlock))]);
}

int Ext2Driver::ReadBlock(const unsigned char** buf, uint32_t blockNum)
{
    if (blockNum > blocksCount && blocksCount != 0) {
        return IBlockDevice::AccessOutOfRange;
    }
    auto result = device->Read(
        this->buf.Get(),
        Mul2N(blockNum, logDeviceBlocksPerBlock),
        Mul2N(1, Max(0, logDeviceBlocksPerBlock))
    );
    if (result != IBlockDevice::NoError) {
        return result;
    }
    *buf = this->buf.Get() + Mul2N(blockNum & blkMask, logBlkSize);
    return IBlockDevice::NoError;
}

uint32_t Ext2Driver::Mul2N(uint32_t val, int pwr)
{
    if (pwr < 0) {
        return val >> -(uint32_t)pwr;
    } else {
        return val << (uint32_t)pwr;
    }
}

uint32_t Ext2Driver::GetBlocksCount() const
{
    return boot_LE32ULoad(&superblock.blocksCount);
}

} // namespace boot::ext2
