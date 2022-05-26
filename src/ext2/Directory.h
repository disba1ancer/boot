#ifndef BOOT_EXT2_DIRECTORY_H
#define BOOT_EXT2_DIRECTORY_H

#include "File.h"

namespace boot {
namespace ext2 {

class Directory;

class DirectoryIterator;

class DirectoryEntry {
    friend class DirectoryIterator;
    DirectoryEntry(
        Driver& driver,
        uint32_t iNode,
        ::ext2::FileType type,
        UniquePtr<char[]>&& name,
        size_t nameLen
    );
public:
    File Open() const;
    auto Name() const -> const char*;
    auto NameLen() const -> size_t;
    auto Type() const -> ::ext2::FileType;
private:
    Driver* driver;
    uint32_t iNode;
    ::ext2::FileType type;
    UniquePtr<char[]> name;
    size_t nameLen;
};

class DirectoryIterator {
    friend class Directory;
    DirectoryIterator(Directory& dir, uint64_t pos);
public:
    void Next();
    bool IsEqual(const DirectoryIterator& oth) const;
    auto Entry() -> DirectoryEntry;
    auto operator++() -> DirectoryIterator&;
    bool operator!=(const DirectoryIterator& oth) const;
    auto operator*() -> DirectoryEntry;
private:
    Directory* dir;
    uint64_t pos;
    unsigned int next;
};

class Directory {
public:
    friend class DirectoryEntry;
    friend class DirectoryIterator;
    explicit Directory(File&& file);
    DirectoryIterator begin();
    DirectoryIterator end();
private:
    File file;
};

} // namespace ext2
} // namespace boot

#endif // BOOT_EXT2_DIRECTORY_H
