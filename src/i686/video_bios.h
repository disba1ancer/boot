#ifndef VIDEO_BIOS_H
#define VIDEO_BIOS_H

#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

enum i686_VideoBIOS_WrStrMode {
    i686_VideoBIOS_WrStrMode_UpdateCursor = 1,
    i686_VideoBIOS_WrStrMode_WithAttribute = 2,
};

typedef struct i686_VideoBIOS_VideoModeInfo {
    alignas(2)
    uint8_t mode;
    uint8_t cols;
    uint8_t page;
} i686_VideoBIOS_VideoModeInfo;

typedef struct i686_VideoBIOS_CursorInfo {
    alignas(2)
    uint8_t endCursorScanline;
    uint8_t startCursorScanline;
    uint8_t column;
    uint8_t row;
} i686_VideoBIOS_CursorInfo;

#ifdef __cplusplus
extern "C" {
#endif

void i686_VideoBIOS_WriteString(int mode, int pageNum, int color, size_t stringSize, int row, int column, const char *str);
i686_VideoBIOS_VideoModeInfo i686_VideoBIOS_GetVideoMode(void);
i686_VideoBIOS_CursorInfo i686_VideoBIOS_GetCursorPosSize(void);

void i686_vbe_GetInformation(void *buf);

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

inline void GetInformation(void *buf) { i686_vbe_GetInformation(buf); }

}

#endif

#endif // VIDEO_BIOS_H
