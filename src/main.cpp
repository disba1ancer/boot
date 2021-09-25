#include "i686/processor.h"
#include "i686/video_bios.h"
#include "i686/membios.h"
#include <cstddef>
#include <cstdlib>
#include <stdlib.h>

char str[] = "Hello, World!\r\n";

struct GPTPartition {
  unsigned char typeGUID[16];
  unsigned char partGUID[16];
  uint64_t firstLBA;
  uint64_t lastLBA;
  uint64_t attributes;
};

struct StartupInfo {
    uint16_t pnpPtr[2];
    int diskNum;
    GPTPartition part;
};

extern "C" void * heap_start;
extern "C" void * heap_end;

char* u64toha(uint64_t val, char* buf, size_t size) {
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

extern "C" void boot_main(StartupInfo si [[maybe_unused]])
{
    using namespace i686::VideoBIOS;
    auto info = GetVideoMode();
    auto cursor = GetCursorPosSize();
    WriteString(WrStrMode_UpdateCursor, info.page, 7, sizeof(str) - 1, cursor.row, cursor.column, str);
    i686_MemoryMapEntry entry;
    unsigned context = 0;
    int row = 0;
    do {
        i686_GetMemoryMap(&context, &entry);
        char buf[17];
        auto str = u64toha(entry.startRegion, buf, 17);
        WriteString(WrStrMode_UpdateCursor, info.page, 7, std::size_t(buf + sizeof(buf) - str) - 1, row, 0, str);
        str = u64toha(entry.regionSize, buf, 17);
        WriteString(WrStrMode_UpdateCursor, info.page, 7, buf + sizeof(buf) - str - 1, row, 20, str);
        str = u64toha(entry.type, buf, 17);
        WriteString(WrStrMode_UpdateCursor, info.page, 7, buf + sizeof(buf) - str - 1, row, 40, str);
        str = u64toha(entry.flags, buf, 17);
        WriteString(WrStrMode_UpdateCursor, info.page, 7, buf + sizeof(buf) - str - 1, row, 50, str);
        ++row;
    } while (context);
    void *ptr = malloc(0x40001);
    free(ptr);
}
