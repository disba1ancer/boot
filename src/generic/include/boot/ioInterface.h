#ifndef BOOT_IOINTERFACE_H
#define BOOT_IOINTERFACE_H

#include <stddef.h>
#include <stdint.h>

namespace boot {

enum class IOStatus {
    NoError = 0,
    InterfaceErrors = 0xFF00,
    AccessOutOfRange
};

class IBlockDevice {
public:

    virtual auto GetBlockSize() const -> size_t = 0;
    virtual auto GetBlockCount() const -> uint64_t = 0;
    virtual auto Read(void* buf, size_t blkCnt, size_t* readCnt, uint64_t blkNum) -> IOStatus = 0;
};

} // namespace boot

#endif // IBLOCKDEVICE_H
