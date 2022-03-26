#include "i686/processor.h"
#include "i686/bios_video.h"
#include "i686/membios.h"
#include "i686/init.h"
#include "i686/bios_disk.h"
#include "boot/gpt.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include "boot/Conout.hpp"
#include "i686/bios_kbrd.h"
#include "i686/PartitionDevice.h"
#include "ext2/Driver.h"
#include "ext2/File.h"

char str[] = "Hello, World!\n";

char* u64toha(char* buf, size_t size, uint64_t val) {
    static const char map[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    if (size == 0) {
        return buf;
    }
    char* ptr = buf + size - 1;
    *ptr = 0;
    if (size > 1) {
        if (val == 0) {
            *(--ptr) = '0';
            return ptr;
        }
        while (val && ptr != buf) {
            *(--ptr) = map[val & 0xF];
            val >>= 4;
        }
    }
    return ptr;
}

using i686::bios::mem::MapEntry;

extern "C" void boot_main(boot_StartupInfo *si [[maybe_unused]], size_t count, MapEntry *memmap)
{
    using namespace i686::bios;
    auto &out = boot::Conout::instance;
    out.PutStr(str);
    char buf[17];
    for (size_t i = 0; i < count; ++i) {
        auto str = u64toha(buf, 17, memmap[i].startRegion);
        out.PutStr(str);
        out.PutC(' ');
        str = u64toha(buf, 17, memmap[i].regionSize);
        out.PutStr(str);
        out.PutC(' ');
        str = u64toha(buf, 17, memmap[i].type);
        out.PutStr(str);
        out.PutC(' ');
        str = u64toha(buf, 17, memmap[i].flags);
        out.PutStr(str);
        out.PutC('\n');
    }
    while (true) {
        auto key = kbrd::GetKey();
        switch (key.ascii) {
            case 4:
            case 3:
                goto whileEnd;
        }
        out.PutC(key.ascii);
    } whileEnd:
    i686::PartitionDevice part(si->diskNum, &si->part);
    boot::ext2::Driver ext2drv(&part);
    auto file = ext2drv.OpenINode(2);
    file.Read(buf, 1, 17);
    out.Write(buf, 17);
    out.PutC('\n');
}
