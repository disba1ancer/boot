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

auto PartitionDevice::Read(void* buf, uint64_t blkNum, size_t blkCnt) -> boot::IOStatus
{
    auto current = (unsigned char*)buf;
    blkNum += partition->firstLBA;
    // TODO: add block device interface and his error codes
    if (blkNum + blkCnt > partition->lastLBA + 1) {
        return boot::IOStatus::AccessOutOfRange;
    }
    bios::disk::AddressPacket addr = {};
    addr.size = sizeof(addr);
    while (blkCnt != 0) {
        addr.blkCount = (blkCnt < 127) ? blkCnt : 127;
        addr.buffer = i686_MakeRMPointer(current);
        addr.lba = blkNum;
        int r = bios::disk::Read(diskNum, &addr);
        if (r != 0) {
            return boot::IOStatus(r);
        }
        blkCnt -= addr.blkCount;
        blkNum += addr.blkCount;
        current += addr.blkCount * blkSize;
    }
    return boot::IOStatus::NoError;
}

} // namespace i686
