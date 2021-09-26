#ifndef VIDEO_BIOS_H
#define VIDEO_BIOS_H

#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include "bt_util.h"
#include "processor.h"

enum i686_VideoBIOS_WrStrMode {
    i686_VideoBIOS_WrStrMode_UpdateCursor = 1,
    i686_VideoBIOS_WrStrMode_WithAttribute = 2,
};

BOOT_STRUCT(i686_VideoBIOS_VideoModeInfo) {
    alignas(2)
    uint8_t mode;
    uint8_t cols;
    uint8_t page;
};

BOOT_STRUCT(i686_VideoBIOS_CursorInfo) {
    alignas(2)
    uint8_t endCursorScanline;
    uint8_t startCursorScanline;
    uint8_t column;
    uint8_t row;
};

BOOT_STRUCT(i686_vbe_Info) {
    char magic[4];
    uint16_t version;
    i686_FarPtr oemStr;
    uint16_t flagsLow;
    uint16_t flagsHigh;
    i686_FarPtr videoModes;
    uint16_t vmem;
    uint16_t oemSoftVer;
    i686_FarPtr vendorName;
    i686_FarPtr productName;
    i686_FarPtr revisionStr;
    uint16_t verUnkn;
    i686_FarPtr accelVModes;
    unsigned char reserved[216 + 256];
};

#ifdef __cplusplus
extern "C" {
#endif

void i686_VideoBIOS_WriteString(int mode, int pageNum, int color, size_t stringSize, int row, int column, const char *str);
i686_VideoBIOS_VideoModeInfo i686_VideoBIOS_GetVideoMode(void);
i686_VideoBIOS_CursorInfo i686_VideoBIOS_GetCursorPosSize(void);

void i686_vbe_GetInformation(i686_vbe_Info *buf);

#ifdef __cplusplus
} // extern "C"

namespace i686::VideoBIOS {

enum WrStrMode {
    WrStrMode_UpdateCursor = i686_VideoBIOS_WrStrMode_UpdateCursor,
    WrStrMode_WithAttribute = i686_VideoBIOS_WrStrMode_WithAttribute
};
using VideoModeInfo = i686_VideoBIOS_VideoModeInfo;
using CursorInfo = i686_VideoBIOS_CursorInfo;

inline void WriteString(int mode, int pageNum, int color, size_t stringSize, int row, int column, const char *str) /*noexcept*/
{ i686_VideoBIOS_WriteString(mode, pageNum, color, stringSize, row, column, str); }

inline VideoModeInfo GetVideoMode() /*noexcept*/ { return i686_VideoBIOS_GetVideoMode(); }

inline CursorInfo GetCursorPosSize() /*noexcept*/ { return i686_VideoBIOS_GetCursorPosSize(); }

}

namespace i686::vbe {

using Info = i686_vbe_Info;

inline void GetInformation(Info &buf) { i686_vbe_GetInformation(&buf); }

}

#endif

#endif // VIDEO_BIOS_H
