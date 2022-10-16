#include "PartitionDevice.h"
#include "bios_disk.h"

namespace i686 {

PartitionDevice::PartitionDevice(int diskNum, boot::GPTPartition* partition) :
    diskNum(diskNum),
    partition(partition)
{
    bios::disk::DriveParameters3 params;
    params.size = bios::disk::DriveParameters3Size;
    params.flags = 0;
    params.magic = 0;
    bios::disk::GetDriveParameters(diskNum, &params);
    blkSize = params.bytesPerSect;
}

size_t PartitionDevice::GetBlockSize() const
{
    return blkSize;
}

uint64_t PartitionDevice::GetBlockCount() const
{
    return partition->lastLBA - partition->firstLBA + 1;
}

auto PartitionDevice::Read(void* buf, size_t blkCnt, size_t* readCnt, uint64_t startBlk) -> boot::IOStatus
{
    auto cbuf = (unsigned char*)buf;
    startBlk += partition->firstLBA;
    if (readCnt != nullptr) {
        *readCnt = 0;
    }
    if (startBlk > partition->lastLBA) {
        return boot::IOStatus::AccessOutOfRange;
    }
    bios::disk::AddressPacket addr = {};
    addr.size = sizeof(addr);
    auto blkNum = startBlk;
    blkCnt = size_t(boot::Min(blkCnt, partition->lastLBA - blkNum + 1));
    while (blkCnt != 0) {
        addr.blkCount = uint8_t(boot::Min(blkCnt, 127));
        addr.buffer = i686_MakeRMPointer(cbuf);
        addr.lba = blkNum;
        int r = bios::disk::Read(diskNum, &addr);
        if (r != 0) {
            if (readCnt != nullptr) {
                *readCnt = size_t(blkNum + addr.blkCount - startBlk);
            }
            return boot::IOStatus(r);
        }
        blkCnt -= addr.blkCount;
        blkNum += addr.blkCount;
        cbuf += addr.blkCount * blkSize;
    }
    if (readCnt != nullptr) {
        *readCnt = size_t(blkNum + addr.blkCount - startBlk);
    }
    return boot::IOStatus::NoError;
}

} // namespace i686
