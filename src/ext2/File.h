#ifndef BOOT_EXT2_FILE_H
#define BOOT_EXT2_FILE_H

#include <stdint.h>
#include "ext2.h"
#include "boot/UniquePtr.hpp"
#include "boot/ioInterface.h"

namespace boot {
namespace ext2 {

class Driver;

class File
{
    File(Driver* driver, uint32_t iNodeNum);
public:
    friend class Driver;
    auto GetMode() const -> uint32_t;
    auto GetUID() const -> uint32_t;
    auto GetSize() const -> uint64_t;
    auto GetGID() const -> uint32_t;
    bool IsDirectory() const;
    auto Read(void* buf, size_t length, size_t* readCnt, uint64_t start) -> IOStatus;
    auto GetDriver() const -> Driver&;
private:
    auto MapBlock(uint32_t blockNum) -> uint32_t;
    int  ReadBlock(void* buf, uint32_t blockNum);

    Driver* driver;
    ::ext2::INode iNode;
    uint32_t lastBlock;
    int logBlocksPer;
    size_t blockCountPer1;
    size_t blockCountPer2;
    size_t mask1;
    size_t mask2;
    static constexpr size_t cacheLevel = 3;
    UniquePtr<boot_LE32U[]> indexCache[cacheLevel];
    uint32_t lastDataBlock;
    UniquePtr<unsigned char[]> dataCache;
};

} // namespace ext2
} // namespace boot

#endif // BOOT_EXT2_FILE_H
