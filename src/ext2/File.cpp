#include "File.h"
#include "Driver.h"
#include <exception>

namespace boot {
namespace ext2 {

File::File(Driver* driver, uint32_t iNodeNum) :
    driver(driver),
    iNode({})
{
    if (driver->LoadINode(&iNode, iNodeNum) != IBlockDevice::NoError) {
        std::terminate();
    }
}

} // namespace ext2
} // namespace boot
