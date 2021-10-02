#include "i686/processor.h"
#include "i686/bios_video.h"
#include "i686/membios.h"
#include "i686/init.h"
#include "boot/gpt.h"
#include <cstddef>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include "boot/Conout.hpp"
#include "i686/bios_kbrd.h"

char str[] = "Hello, World!\r\n";

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

extern "C" void boot_main(boot_StartupInfo *si [[maybe_unused]])
{
    using namespace i686::VideoBIOS;
    auto &out = boot::Conout::Instance();
    out.PutStr(str);
    i686_MemoryMapEntry entry;
    unsigned context = 0;
    int row = 0;
    do {
        i686_GetMemoryMap(&context, &entry);
        char buf[17];
        auto str = u64toha(buf, 17, entry.startRegion);
        out.PutStr(str);
        out.PutC(' ');
        str = u64toha(buf, 17, entry.regionSize);
        out.PutStr(str);
        out.PutC(' ');
        str = u64toha(buf, 17, entry.type);
        out.PutStr(str);
        out.PutC(' ');
        str = u64toha(buf, 17, entry.flags);
        out.PutStr(str);
        out.PutC('\n');
        ++row;
    } while (context);
    i686::vbe::Info vbeInfo;
    memcpy(vbeInfo.magic, "VBE2", 4);
    i686::vbe::GetInformation(vbeInfo);
    auto modes = i686_LoadPointer<uint16_t>(vbeInfo.videoModes);
    i686::vbe::ModeInfo modeInfo;
    while (*modes != 0xFFFF) {
        i686::vbe::GetModeInfo(*(modes++) | i686::vbe::LFB, modeInfo);
    }
    while (true) {
        using namespace i686::bios::kbrd;
        auto key = GetKey();
        if (key.scanCode == 1) break;
        out.PutC(key.ascii);
    }
}
