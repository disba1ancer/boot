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
#include "boot/Conout.hpp"
#include "platform/i686/bios_kbrd.h"
#include "platform/i686/PartitionDevice.h"
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
    uint64_t pos = 0;
    size_t readSize;
    while (kernel.Read(buf, 1024, &readSize, pos) == boot::IOStatus::NoError) {
        out.Write(buf, boot::Min(1024, readSize));
        pos += readSize;
    }
    out.PutC('\n');
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
