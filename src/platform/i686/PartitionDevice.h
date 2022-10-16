#ifndef I686_PARTITIONDEVICE_H
#define I686_PARTITIONDEVICE_H

#include "boot/gpt.h"
#include "boot/ioInterface.h"

namespace i686 {

struct PartitionDevice final : boot::IBlockDevice
{
    PartitionDevice(int diskNum, boot::GPTPartition* partition);
    auto GetBlockSize() const -> size_t override;
    auto GetBlockCount() const -> uint64_t override;
    auto Read(void* buf, size_t blkCnt, size_t* readCnt, uint64_t blkNum) -> boot::IOStatus override;
private:
    int diskNum;
    size_t blkSize;
    boot::GPTPartition* partition;
};

} // namespace i686

#endif // I686_PARTITIONDEVICE_H
