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
#include "ext2/Directory.h"
#include "boot/util.h"

char str[] = "Hello, World!\n";

using i686::bios::mem::MapEntry;

extern "C" void boot_main(boot_StartupInfo *si [[maybe_unused]], size_t count, MapEntry *memmap)
{
    using namespace i686::bios;
    auto &out = boot::Conout::instance;
    out.PutStr(str);
    char buf[17];
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
    boot::ext2::Directory dir(ext2drv.OpenINode(2));
    for (auto&& entry : dir) {
        out.PutStr(entry.Name());
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
    } whileEnd:;
}
