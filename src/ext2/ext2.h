#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>
#include "boot/endian.h"

enum ext2_ {
    ext2_ValidFS = 1,
    ext2_ErrorFS = 2,
    ext2_SuperblockMagic = 0xEF53,
    ext2_GoodOldRev = 0,
    ext2_GoodOldINodeSize = 128,
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

enum ext2_INodes {
    ext2_BadINode = 1,
    ext2_RootINode = 2,
    ext2_AclIdxINode = 3,
    ext2_AclDataINode = 4,
    ext2_BootINode = 5,
    ext2_UnderDirINode = 6,
};

enum ext2_Mode {
    ext2_Mode_TypeMask = 0xF000,
    ext2_Mode_Sock = 0xC000,
    ext2_Mode_SymLink = 0xA000,
    ext2_Mode_File = 0x8000,
    ext2_Mode_BlkDev = 0x6000,
    ext2_Mode_Dir = 0x4000,
    ext2_Mode_ChrDev = 0x2000,
    ext2_Mode_FIFO = 0x1000,

    ext2_Mode_UID = 04000,
    ext2_Mode_GID = 02000,
    ext2_Mode_Sticky = 01000,
    ext2_Mode_RUsr = 0400,
    ext2_Mode_WUsr = 0200,
    ext2_Mode_XUsr = 0100,
    ext2_Mode_RGrp =  040,
    ext2_Mode_WGrp =  020,
    ext2_Mode_XGrp =  010,
    ext2_Mode_ROth =   04,
    ext2_Mode_WOth =   02,
    ext2_Mode_XOth =   01,
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

BOOT_STRUCT(ext2_INode) {
    boot_LE16U mode;
    boot_LE16U uid;
    boot_LE32U sizeLow;
    boot_LE32U accessTime;
    boot_LE32U createTime;
    boot_LE32U modifyTime;
    boot_LE32U deleteTime;
    boot_LE16U gid;
    boot_LE16U linksCount;
    boot_LE32U sectors;
    boot_LE32U flags;
    boot_LE32U osd1;
    union {
        unsigned char data[60];
        boot_LE32U blocks[15];
    };
    boot_LE32U generation;
    boot_LE32U fileAcl;
    boot_LE32U dirAcl;
    boot_LE32U fragAddr;
    uint8_t    fragNum;
    uint8_t    fragSize;
    boot_LE16U modeHigh;
    boot_LE16U uidHigh;
    boot_LE16U gidHigh;
    boot_LE32U author;
};

BOOT_STRUCT(ext2_GroupDesc) {
    boot_LE32U blockBitmap;
    boot_LE32U iNodeBitmap;
    boot_LE32U iNodeTable;
    boot_LE16U freeBlocksCount;
    boot_LE16U freeINodesCount;
    boot_LE16U dirAllocBlocks;
    char pad[14];
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
NS_USING(ext2_, GroupDesc)
NS_USING(ext2_, INode)

NS_USING(ext2_, INodes)
NS_CONST(ext2_, BadINode)
NS_CONST(ext2_, RootINode)
NS_CONST(ext2_, AclIdxINode)
NS_CONST(ext2_, AclDataINode)
NS_CONST(ext2_, BootINode)
NS_CONST(ext2_, UnderDirINode)

NS_USING(ext2_, Mode)

NS_CONST(ext2_, Mode_TypeMask)
NS_CONST(ext2_, Mode_Sock)
NS_CONST(ext2_, Mode_SymLink)
NS_CONST(ext2_, Mode_File)
NS_CONST(ext2_, Mode_BlkDev)
NS_CONST(ext2_, Mode_Dir)
NS_CONST(ext2_, Mode_ChrDev)
NS_CONST(ext2_, Mode_FIFO)

NS_CONST(ext2_, Mode_UID)
NS_CONST(ext2_, Mode_GID)
NS_CONST(ext2_, Mode_Sticky)
NS_CONST(ext2_, Mode_RUsr)
NS_CONST(ext2_, Mode_WUsr)
NS_CONST(ext2_, Mode_XUsr)
NS_CONST(ext2_, Mode_RGrp)
NS_CONST(ext2_, Mode_WGrp)
NS_CONST(ext2_, Mode_XGrp)
NS_CONST(ext2_, Mode_ROth)
NS_CONST(ext2_, Mode_WOth)
NS_CONST(ext2_, Mode_XOth)

#undef NS_CONST
#undef NS_USING

} // namespace ext2

#endif

#endif // EXT2_H
