#ifndef IBLOCKDEVICE_H
#define IBLOCKDEVICE_H

#include <stddef.h>
#include <stdint.h>

namespace boot {

class IBlockDevice
{
public:
    enum BlockDeviceError {
        NoError = 0,
        InterfaceErrors = 0xFF00,
        AccessOutOfRange
    };

    virtual auto GetBlockSize() -> size_t = 0;
    virtual auto GetBlockCount() -> uint64_t = 0;
    virtual auto Read(unsigned char* buf, uint64_t blkNum, size_t blkCnt) -> BlockDeviceError = 0;
};

} // namespace boot

#endif // IBLOCKDEVICE_H
