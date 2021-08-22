#include "i686/processor.h"
#include "i686/video_bios.h"

char str[] = "Hello, World!\r\n";

extern "C" void boot_main()
{
    using namespace I686::VideoBIOS;
    auto info = GetVideoMode();
    auto cursor = GetCursorPosSize();
    WriteString(WrStrMode_UpdateCursor, info.page, 7, sizeof(str) - 1, cursor.row, cursor.column, str);
}
