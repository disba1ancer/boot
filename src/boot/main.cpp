#include "platform/i686/processor.h"
#include "platform/i686/bios_video.h"
#include "platform/i686/membios.h"
#include "platform/i686/init.h"
#include "platform/i686/bios_disk.h"
#include "boot/gpt.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include "boot/Conout.hpp"
#include "platform/i686/bios_kbrd.h"
#include "platform/i686/PartitionDevice.h"
#include "ext2/Driver.h"
#include "ext2/File.h"
#include "ext2/Directory.h"
#include "boot/util.h"
#include "boot/virtual_alloc.h"
#include "elf/elf.h"

char str[] = "Hello, World!\n";

using i686::bios::mem::MapEntry;

extern "C" void boot_main(boot_StartupInfo *si [[maybe_unused]], size_t count, MapEntry *memmap)
{
    using namespace i686::bios;
    auto &out = boot::Conout::instance;
    out.PutStr(str);
    char buf[1024];
    for (size_t i = 0; i < count; ++i) {
        out.PutStr("0x");
        boot::UToStr(buf, 17, memmap[i].startRegion, 16);
        out.PutStr(buf);
        out.PutStr(" 0x");
        boot::UToStr(buf, 17, memmap[i].regionSize, 16);
        out.PutStr(buf);
        out.PutStr(" 0x");
        boot::UToStr(buf, 17, memmap[i].type, 16);
        out.PutStr(buf);
        out.PutStr(" 0x");
        boot::UToStr(buf, 17, memmap[i].flags, 16);
        out.PutStr(buf);
        out.PutC('\n');
    }
    i686::PartitionDevice part(si->diskNum, &si->part);
    boot::ext2::Driver ext2drv(&part);
    auto kernel = ext2drv.OpenByPath("/bin/kernel");
    size_t readSize;
    elf_ident ident;
    kernel.Read(&ident, sizeof(ident), &readSize, 0);
    constexpr byte elf_magic[] = { 0x7F, 'E', 'L', 'F' };
    if (
        memcmp(ident.magic, elf_magic, 4) != 0 ||
        ident.fileClass != elf_Class_C64 ||
        ident.data != elf_DataEnc_LSB
    ) {
        std::terminate();
    }
    static constexpr auto e = boot::Endian_Little;
    elf64_Header header;
    kernel.Read(&header, sizeof(header), &readSize, sizeof(ident));
    if (
        boot::ELoad(header.type, e) != elf_Type_Executable ||
        boot::ELoad(header.machine,e) != elf_Machine_x86_64
    ) {
        std::terminate();
    }
    static constexpr auto PageSize = 4096;
    boot::UniquePtr<byte[]> pageBuffer = new byte[PageSize];
    for (unsigned i = 0; i < boot::ELoad(header.progHeadEntryCount, e); ++i) {
        elf64_ProgramHeader programHeader;
        kernel.Read(
            &programHeader,
            sizeof(programHeader),
            &readSize,
            boot::ELoad(header.programHeaderOff, e)
                + boot::ELoad(header.progHeadEntrySize, e) * i
        );
        uint64_t segmentFileStart = boot::ELoad(programHeader.offset, e);
        uint64_t segmentFileEnd = boot::ELoad(programHeader.size, e)
            + segmentFileStart;
        segmentFileStart &= ~(uint64_t)(PageSize - 1);
        uint64_t segmentLoadStart = boot::ELoad(programHeader.virtualAddress, e);
        uint64_t segmentLoadEnd = boot::ELoad(programHeader.loadSize, e)
            + segmentLoadStart;
        segmentLoadStart &= ~(uint64_t)(PageSize - 1);
        int flags = (boot::ELoad(header.flags, e) & 7) + boot_MemoryFlags_Kernel;
        void* dst;
        if (boot::ELoad(programHeader.size, e) > 0) {
            for (;
                segmentFileStart < segmentFileEnd;
                segmentFileStart += PageSize, segmentLoadStart += PageSize
            ) {
                dst = boot_VirtualAlloc(segmentLoadStart, flags);
                if (dst == nullptr) {
                    std::terminate();
                }
                kernel.Read(dst, PageSize, &readSize, segmentFileStart);
                memset((byte *)dst + readSize, 0, PageSize - readSize);
            }
            segmentFileEnd &= (PageSize - 1);
            memset((byte *)dst + segmentFileEnd, 0, PageSize - segmentFileEnd);
        }
        for (;
            segmentLoadStart < segmentLoadEnd;
            segmentLoadStart += PageSize
        ) {
            dst = boot_VirtualAlloc(segmentLoadStart, flags);
            if (dst == nullptr) {
                std::terminate();
            }
            memset(dst, 0, PageSize);
        }
    }
    boot_VirtualEnter(boot::ELoad(header.entry, e));
    while (true) {
        auto key = kbrd::GetKey();
        switch (key.ascii) {
            case 4:
            case 3:
                goto whileEnd;
        }
        out.PutC(key.ascii);
    } whileEnd:;
}
