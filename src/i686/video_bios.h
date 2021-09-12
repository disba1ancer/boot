#ifndef VIDEO_BIOS_H
#define VIDEO_BIOS_H

#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

enum I686_VideoBIOS_WrStrMode {
    I686_VideoBIOS_WrStrMode_UpdateCursor = 1,
    I686_VideoBIOS_WrStrMode_WithAttribute = 2,
};

typedef struct I686_VideoBIOS_VideoModeInfo {
    alignas(2)
    uint8_t mode;
    uint8_t cols;
    uint8_t page;
} I686_VideoBIOS_VideoModeInfo;

typedef struct I686_VideoBIOS_CursorInfo {
    alignas(2)
    uint8_t endCursorScanline;
    uint8_t startCursorScanline;
    uint8_t column;
    uint8_t row;
} I686_VideoBIOS_CursorInfo;

#ifdef __cplusplus
extern "C" {
#endif

void I686_VideoBIOS_WriteString(int mode, int pageNum, int color, size_t stringSize, int row, int column, char *str);
I686_VideoBIOS_VideoModeInfo I686_VideoBIOS_GetVideoMode(void);
I686_VideoBIOS_CursorInfo I686_VideoBIOS_GetCursorPosSize(void);

#ifdef __cplusplus
} // extern "C"

namespace I686::VideoBIOS {

enum WrStrMode {
    WrStrMode_UpdateCursor = I686_VideoBIOS_WrStrMode_UpdateCursor,
    WrStrMode_WithAttribute = I686_VideoBIOS_WrStrMode_WithAttribute
};
using VideoModeInfo = I686_VideoBIOS_VideoModeInfo;
using CursorInfo = I686_VideoBIOS_CursorInfo;

inline void WriteString(int mode, int pageNum, int color, size_t stringSize, int row, int column, char *str) /*noexcept*/
{ I686_VideoBIOS_WriteString(mode, pageNum, color, stringSize, row, column, str); }

inline VideoModeInfo GetVideoMode() /*noexcept*/ { return I686_VideoBIOS_GetVideoMode(); }

inline CursorInfo GetCursorPosSize() /*noexcept*/ { return I686_VideoBIOS_GetCursorPosSize(); }

}

#endif

#endif // VIDEO_BIOS_H
