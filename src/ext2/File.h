#ifndef BOOT_EXT2_FILE_H
#define BOOT_EXT2_FILE_H

#include <stdint.h>
#include "ext2.h"
#include "boot/UniquePtr.hpp"

namespace boot {
namespace ext2 {

class Driver;

class File
{
    Driver* driver;
    ::ext2::INode iNode;
    uint32_t lastBlock;
    static constexpr size_t cacheLevel = 3;
    UniquePtr<boot_LE32U[]> indexCache[cacheLevel];
    UniquePtr<unsigned char[]> dataCache;

    File(Driver* driver, uint32_t iNodeNum);
public:
    friend class Driver;
    auto GetMode() const -> uint32_t;
    auto GetUID() const -> uint32_t;
    auto GetSize() const -> uint64_t;
    auto GetGID() const -> uint32_t;
    bool IsDirectory() const;
    int  Read(void* buf, uint64_t start, size_t length);
    auto MapBlock(uint32_t blockNum) -> uint32_t;
private:
    int  ReadBlock(void* buf, uint32_t blockNum);
};

} // namespace ext2
} // namespace boot

#endif // BOOT_EXT2_FILE_H
