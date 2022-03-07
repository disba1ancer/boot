#ifndef BOOT_EXT2_FILE_H
#define BOOT_EXT2_FILE_H

#include <stdint.h>
#include "ext2.h"

namespace boot {
namespace ext2 {

class Driver;

class File
{
public:
    friend class Driver;
private:
    File(Driver* driver, uint32_t iNodeNum);
    Driver* driver;
    ::ext2::INode iNode;
};

} // namespace ext2
} // namespace boot

#endif // BOOT_EXT2_FILE_H
