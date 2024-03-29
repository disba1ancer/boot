#include "Driver.h"
#include "File.h"
#include <exception>
#include "boot/util.h"
#include "boot/util.hpp"
#include "string.h"
#include "boot/Conout.hpp"
#include "Directory.h"

namespace boot::ext2 {

Driver::Driver(IBlockDevice* device) :
    device(device),
    superblock({})
{
    ReinitBuffer(0);
    if (ReadBlock(buf.Get(), 1) != int(IOStatus::NoError)) {
        std::terminate();
    }
    memcpy(&superblock, buf.Get(), sizeof(superblock));
    if (ELoad(superblock.magic) != ext2_SuperblockMagic) {
        std::terminate();
    }
    ReinitBuffer(ELoad(superblock.logBlockSize));
}

size_t Driver::GetBlockSize() const
{
    return Mul2N(1, LogBlockSize());
}

int Driver::LogBlockSize() const
{
    return int(ELoad(superblock.logBlockSize)) + 10;
}

void Driver::ReinitBuffer(unsigned logBlockSize)
{
    auto devBlockSize = device->GetBlockSize();
    auto logDevBlkSz = boot_Log2U64(devBlockSize);
    if ((1U << logDevBlkSz) != devBlockSize) {
        std::terminate();
    }
    logSectorsPerBlock = 10 + int(logBlockSize) - logDevBlkSz;
    blkMask = 0;
    if (logSectorsPerBlock < 0) {
        blkMask = (uint32_t(1) << uint32_t(-logSectorsPerBlock)) - 1;
    }
    bufStartSect = InvalidSector;
    buf.Reset();
    if (logSectorsPerBlock < 0) {
        buf.Reset(new unsigned char[devBlockSize + Mul2N(1024, int(logBlockSize))]);
    } else {
        buf.Reset(new unsigned char[Mul2N(devBlockSize, logSectorsPerBlock)]);
    }
}

int Driver::ReadBlock(void* buf, uint32_t blockNum)
{
    auto blocksCount = GetBlocksCount();
    if (blockNum > blocksCount && blocksCount != 0) {
        return int(IOStatus::AccessOutOfRange);
    }
    if (logSectorsPerBlock < 0) {
        return ReadBlockSpecial(buf, blockNum);
    }
    auto result = device->Read(
        buf,
        Mul2N(1, Max(0, logSectorsPerBlock)),
        nullptr,
        Mul2N(blockNum, logSectorsPerBlock)
    );
    if (result != IOStatus::NoError) {
        return int(result);
    }
    return int(IOStatus::NoError);
}

int Driver::ReadBlockSpecial(void* buf, uint32_t blockNum)
{
    auto startSect = Mul2N(blockNum, logSectorsPerBlock);
    if (startSect != bufStartSect) {
        auto result = device->Read(
            this->buf.Get() + GetBlockSize(),
            Mul2N(1, Max(0, logSectorsPerBlock)),
            nullptr,
            startSect
        );
        if (result != IOStatus::NoError) {
            bufStartSect = InvalidSector;
            return int(result);
        }
        bufStartSect = startSect;
    }
    memcpy(
        buf,
        this->buf.Get() + (blockNum & blkMask) * GetBlockSize(),
        GetBlockSize()
    );
    return int(IOStatus::NoError);
}

uint32_t Driver::Mul2N(uint32_t val, int pwr)
{
    if (pwr < 0) {
        return val >> (uint32_t)(-pwr);
    } else {
        return val << (uint32_t)pwr;
    }
}

uint32_t Driver::GetBlocksCount() const
{
    return ELoad(superblock.blocksCount);
}

uint32_t Driver::GetRevisionLevel() const
{
    return ELoad(superblock.revisionLevel);
}

size_t Driver::GetINodeSize() const
{
    if (GetRevisionLevel() == ext2_GoodOldRev) {
        return ext2_GoodOldINodeSize;
    } else {
        return ELoad(superblock.iNodeSize);
    }
    return 0;
}

uint32_t Driver::GetINodesPerGroup() const
{
    return ELoad(superblock.iNodesPerGroup);
}

File Driver::OpenByINode(uint32_t iNode)
{
    return { this, iNode };
}

namespace {

struct PathToken {
    PathToken(const char* name) :
        tokBegin(name)
    {
        for (; *tokBegin == '/'; ++tokBegin) {}
        tokEnd = strchr(tokBegin, '/');
        if (tokEnd == nullptr) tokEnd = tokBegin + strlen(tokBegin);
    }
    const char* Get() const {
        return tokBegin;
    }
    size_t Size() const {
        return size_t(tokEnd - tokBegin);
    }
    bool IsEnd() const {
        return tokBegin == tokEnd;
    }
    void Next() {
        tokBegin = tokEnd;
        for (; *tokBegin == '/'; ++tokBegin) {}
        tokEnd = strchr(tokBegin, '/');
        if (tokEnd == nullptr) tokEnd = tokBegin + strlen(tokBegin);
    }
private:
    const char *tokBegin;
    const char *tokEnd;
};

}

File Driver::OpenByPath(const char* name)
{
    File nextFile = OpenByINode(::ext2::RootINode);
    for (PathToken tok = {name}; !tok.IsEnd(); tok.Next()) {
        Directory curDir(boot::Move(nextFile));
        auto strSize = tok.Size();
        for (auto&& entry : curDir) {
            if (strSize != entry.NameLen()) {
                continue;
            }
            if (memcmp(entry.Name(), tok.Get(), strSize) == 0) {
                nextFile = entry.Open();
                if ( tok.Get()[strSize] == 0 ) {
                    return nextFile;
                }
                goto cont;
            }
        }
        std::terminate();
        cont:;
    }
    std::terminate();
}

int Driver::LoadGroupDesc(::ext2::GroupDesc* desc, uint32_t grpNum)
{
    auto logGroupDescPerBlock = int(ELoad(superblock.logBlockSize)) + 5;
    auto blk = grpNum >> logGroupDescPerBlock;
    int result = ReadBlock(buf.Get(), ELoad(superblock.firstDataBlock) + 1 + blk);
    if (result != int(IOStatus::NoError)) {
        return result;
    }
    auto grpOffset = (grpNum - (blk << logGroupDescPerBlock)) << 5;
    memcpy(desc, buf.Get() + grpOffset, sizeof(::ext2::GroupDesc));
    return int(IOStatus::NoError);
}

int Driver::LoadINode(::ext2::INode* desc, uint32_t iNodeNum)
{
    if (iNodeNum == 0 || iNodeNum > ELoad(superblock.iNodesCount)) {
        return int(IOStatus::AccessOutOfRange);
    }
    auto iNodeGroup = (iNodeNum - 1) / GetINodesPerGroup();
    auto iNodeInGroup = (iNodeNum - 1) % GetINodesPerGroup();
    ::ext2::GroupDesc group;
    auto result = LoadGroupDesc(&group, iNodeGroup);
    if (result != int(IOStatus::NoError)) {
        return result;
    }
    auto logINodesPerBlock = int(ELoad(superblock.logBlockSize))
        + 10 - boot_Log2U32(GetINodeSize());
    auto iNodeBlock = iNodeInGroup >> logINodesPerBlock;
    auto iNodeOffset = (iNodeInGroup - (iNodeBlock << logINodesPerBlock)) * GetINodeSize();
    iNodeBlock += ELoad(group.iNodeTable);
    result = ReadBlock(buf.Get(), iNodeBlock);
    if (result != int(IOStatus::NoError)) {
        return result;
    }
    memcpy(desc, buf.Get() + iNodeOffset, boot::Min(sizeof(::ext2::INode), GetINodeSize()));
    return int(IOStatus::NoError);
}

} // namespace boot::ext2
