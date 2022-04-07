#include "Directory.h"
#include "Driver.h"
#include "boot/util.hpp"
#include <exception>

namespace boot {
namespace ext2 {

Directory::Directory(File&& file) :
    file(boot::Move(file))
{}

DirectoryIterator Directory::begin()
{
    return { *this, 0 };
}

DirectoryIterator Directory::end()
{
    return { *this, file.GetSize() };
}

DirectoryIterator::DirectoryIterator(Directory& dir, uint64_t pos) :
    dir(&dir),
    pos(pos),
    next(0)
{}

void DirectoryIterator::Next()
{
    if (next > 0) {
        pos += next;
        next = 0;
    } else {
        ::ext2::Directory dirEnt;
        dir->file.Read(&dirEnt, pos, sizeof(dirEnt));
        pos += boot::ELoad(dirEnt.length);
    }
}

bool DirectoryIterator::IsEqual(const DirectoryIterator& oth) const
{
    return dir == oth.dir && pos == oth.pos;
}

DirectoryEntry DirectoryIterator::Entry()
{
    ::ext2::Directory dirEnt;
    if (dir->file.Read(&dirEnt, pos, sizeof(dirEnt)) != IBlockDevice::NoError) {
        std::terminate();
    }
    next = boot::ELoad(dirEnt.length);
    UniquePtr<char[]> name = new char[dirEnt.nameLen + 1];
    name[dirEnt.nameLen] = 0;
    if (
        dir->file.Read(name.Get(), pos + sizeof(dirEnt), dirEnt.nameLen) !=
            IBlockDevice::NoError
    ) {
        std::terminate();
    }
    return {
        dir->file.GetDriver(),
        boot::ELoad(dirEnt.iNode),
        ::ext2::FileType(dirEnt.type),
        boot::Move(name),
        dirEnt.nameLen,
    };
}

DirectoryIterator& DirectoryIterator::operator++()
{
    Next();
    return *this;
}

bool DirectoryIterator::operator!=(const DirectoryIterator& oth) const
{
    return !IsEqual(oth);
}

DirectoryEntry DirectoryIterator::operator*()
{
    return Entry();
}

DirectoryEntry::DirectoryEntry(
    Driver& driver,
    uint32_t iNode,
    ::ext2::FileType type,
    UniquePtr<char[]>&& name,
    size_t nameLen
) :
    driver(&driver),
    iNode(iNode),
    type(type),
    name(boot::Move(name)),
    nameLen(nameLen)
{}

File DirectoryEntry::Open() const
{
    return driver->OpenINode(iNode);
}

const char* DirectoryEntry::Name() const
{
    return name.Get();
}

::ext2::FileType DirectoryEntry::Type() const
{
    return type;
}

} // namespace ext2
} // namespace boot
