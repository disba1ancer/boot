#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>
#include "boot/endian.h"

enum ext2_ {
    ext2_ValidFS = 1,
    ext2_ErrorFS = 2,
    ext2_SuperblockMagic = 0xEF53,
    ext2_GoodOldRev = 0,
    ext2_DynamicRev = 1,
};

enum ext2_Errors {
    ext2_Errors_Continue = 1,
    ext2_Errors_RO = 2,
    ext2_Errors_PANIC = 3,
};

enum ext2_OS {
    ext2_OS_Linux = 0,
    ext2_OS_HURD = 1,
    ext2_OS_MASIX = 2,
    ext2_OS_FreeBSD = 3,
    ext2_OS_Lites = 4,
};

enum ext2_FeatureCompat {
    ext2_FeatureCompat_DirectoryPrealloc = 1,
    ext2_FeatureCompat_ImagicINodes = 2,
    ext2_FeatureCompat_HasJournal = 4,
    ext2_FeatureCompat_ExtendedAttributes = 8,
    ext2_FeatureCompat_ResizedINode = 0x10,
    ext2_FeatureCompat_DirectoryIndex = 0x20,
};

enum ext2_FeatureIncompat {
    ext2_FeatureIncompat_Compression = 1,
    ext2_FeatureIncompat_Filetype = 2,
    ext2_FeatureIncompat_Recover = 4,
    ext2_FeatureIncompat_JournalDev = 8,
    ext2_FeatureIncompat_MetaBG = 0x10,
};

enum ext2_FeatureROCompat {
    ext2_FeatureROCompat_SparseSuperblock = 1,
    ext2_FeatureROCompat_LargeFile = 2,
    ext2_FeatureROCompat_BTreeDir = 4,
};

enum ext2_Algorithm {
    ext2_Algorithm_LZV1 = 1,
    ext2_Algorithm_LZRW3A = 2,
    ext2_Algorithm_GZIP = 4,
    ext2_Algorithm_BZIP2 = 8,
    ext2_Algorithm_LZO = 0x10,
};

BOOT_STRUCT(ext2_Superblock) {
    boot_LE32U iNodesCount;
    boot_LE32U blocksCount;
    boot_LE32U reservedBlocksCount;
    boot_LE32U freeBlocksCount;
    boot_LE32U freeINodesCount;
    boot_LE32U firstDataBlock;
    boot_LE32U logBlockSize;
    boot_LE32S logFragSize;
    boot_LE32U blocksPerGroup;
    boot_LE32U fragsPerGroup;
    boot_LE32U iNodesPerGroup;
    boot_LE32U mountTime;
    boot_LE32U writeTime;
    boot_LE16U mountCounter;
    boot_LE16U maxMountCounter;
    boot_LE16U magic;
    boot_LE16U state;
    boot_LE16U errors;
    boot_LE16U minorRevisionLevel;
    boot_LE32U lastCheckTime;
    boot_LE32U checkInterval;
    boot_LE32U creatorOS;
    boot_LE32U revisionLevel;
    boot_LE16U reservedDefUID;
    boot_LE16U reservedDefGID;
    boot_LE32U firstINode;
    boot_LE16U iNodeSize;
    boot_LE16U blockGroupNumber;
    boot_LE32U featureCompat;
    boot_LE32U featureIncompat;
    boot_LE32U featureROCompat;
    unsigned char uuid[16];
    char       volumeName[16];
    char       lastMountPoint[64];
    boot_LE32U algoBitmap;
    uint8_t    preallocBlocks;
    uint8_t    preallocDirectoryBlocks;
    alignas(4)
    unsigned char journalUUID[16];
    boot_LE32U journalINode;
    boot_LE32U journalDev;
    boot_LE32U lastOrphan;
    boot_LE32U hashSeed[4];
    uint8_t    defaultHashVersion;
    alignas(4)
    boot_LE32U defaultMountOptions;
    boot_LE32U firstMetaBG;
};

#ifdef __cplusplus

namespace ext2 {

#define NS_USING(prefix, name) using name = :: prefix ## name;
#define NS_CONST(prefix, name) inline constexpr auto name = :: prefix ## name;

NS_CONST(ext2_, ValidFS)
NS_CONST(ext2_, ErrorFS)
NS_CONST(ext2_, SuperblockMagic)
NS_CONST(ext2_, GoodOldRev)
NS_CONST(ext2_, DynamicRev)

NS_USING(ext2_, Errors)

NS_CONST(ext2_, Errors_Continue)
NS_CONST(ext2_, Errors_RO)
NS_CONST(ext2_, Errors_PANIC)


NS_USING(ext2_, OS)

NS_CONST(ext2_, OS_Linux)
NS_CONST(ext2_, OS_HURD)
NS_CONST(ext2_, OS_MASIX)
NS_CONST(ext2_, OS_FreeBSD)
NS_CONST(ext2_, OS_Lites)

NS_USING(ext2_, FeatureCompat)

NS_CONST(ext2_, FeatureCompat_DirectoryPrealloc)
NS_CONST(ext2_, FeatureCompat_ImagicINodes)
NS_CONST(ext2_, FeatureCompat_HasJournal)
NS_CONST(ext2_, FeatureCompat_ExtendedAttributes)
NS_CONST(ext2_, FeatureCompat_ResizedINode)
NS_CONST(ext2_, FeatureCompat_DirectoryIndex)


NS_USING(ext2_, FeatureIncompat)

NS_CONST(ext2_, FeatureIncompat_Compression)
NS_CONST(ext2_, FeatureIncompat_Filetype)
NS_CONST(ext2_, FeatureIncompat_Recover)
NS_CONST(ext2_, FeatureIncompat_JournalDev)
NS_CONST(ext2_, FeatureIncompat_MetaBG)

NS_USING(ext2_, FeatureROCompat)

NS_CONST(ext2_, FeatureROCompat_SparseSuperblock)
NS_CONST(ext2_, FeatureROCompat_LargeFile)
NS_CONST(ext2_, FeatureROCompat_BTreeDir)

NS_USING(ext2_, Algorithm)

NS_CONST(ext2_, Algorithm_LZV1)
NS_CONST(ext2_, Algorithm_LZRW3A)
NS_CONST(ext2_, Algorithm_GZIP)
NS_CONST(ext2_, Algorithm_BZIP2)
NS_CONST(ext2_, Algorithm_LZO)

NS_USING(ext2_, Superblock)

#undef NS_CONST
#undef NS_USING

} // namespace ext2

#endif

#endif // EXT2_H
