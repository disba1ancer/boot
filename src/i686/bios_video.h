#ifndef VIDEO_BIOS_H
#define VIDEO_BIOS_H

#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include "boot/util.h"
#include "processor.h"

enum i686_bios_video_WrStrMode {
    i686_VideoBIOS_WrStrMode_UpdateCursor = 1,
    i686_VideoBIOS_WrStrMode_WithAttribute = 2,
};

enum i686_bios_video_CursorMode {
    i686_VideoBIOS_CursorMode_Normal = 0,
    i686_VideoBIOS_CursorMode_Invisible = 1 << 5,
    i686_VideoBIOS_CursorMode_Mask = 3 << 5,
};

BOOT_STRUCT(i686_bios_video_ModeInfo) {
    alignas(2)
    uint8_t mode;
    uint8_t cols;
    uint8_t page;
};

BOOT_STRUCT(i686_bios_video_CursorInfo) {
    alignas(2)
    uint8_t endCursorScanline;
    uint8_t startCursorScanline;
    uint8_t column;
    uint8_t row;
};

BOOT_STRUCT(i686_vbe_Info) {
    char magic[4];
    uint16_t version;
    i686_RMPtr oemStr;
    uint16_t flagsLow;
    uint16_t flagsHigh;
    i686_RMPtr videoModes;
    uint16_t vmem;
    uint16_t oemSoftVer;
    i686_RMPtr vendorName;
    i686_RMPtr productName;
    i686_RMPtr revisionStr;
    uint16_t verUnkn;
    i686_RMPtr accelVModes;
    unsigned char reserved[216 + 256];
};

BOOT_STRUCT(i686_vbe_ModeInfo) {
    uint16_t attributes;
    uint8_t windowAAttrs;
    uint8_t windowBAttrs;
    uint16_t windowGranularity;
    uint16_t windowSize;
    uint16_t windowASegment;
    uint16_t windowBSegment;
    uint32_t bankSwitchFunc;
    uint16_t bytesPerScanline;
    uint16_t width;
    uint16_t height;
    uint8_t charWidth;
    uint8_t charHeight;
    uint8_t memPlanesNum;
    uint8_t bpp;
    uint8_t banksNum;
    uint8_t memModelType;
    uint8_t sizeOfBank;
    uint8_t imagePagesNum;
    uint8_t rsv;
    uint8_t redMaskSize;
    uint8_t redFieldPos;
    uint8_t greenMaskSize;
    uint8_t greenFieldPos;
    uint8_t blueMaskSize;
    uint8_t blueFieldPos;
    uint8_t rsvMaskSize;
    uint8_t rsvMaskPos;
    uint8_t directColorMode;
    volatile void *lfbAddr;
    volatile void *offscreenStart;
    uint16_t offscreenMemorySize;
    uint16_t linearBytesPerScanline;
    uint8_t numberOfBankedImages;
    uint8_t numberOfLinearImages;
    uint8_t directRedMaskSize;
    uint8_t directRedFieldPos;
    uint8_t directGreenMaskSize;
    uint8_t directGreenFieldPos;
    uint8_t directBlueMaskSize;
    uint8_t directBlueFieldPos;
    uint8_t directRsvMaskSize;
    uint8_t directRsvMaskPos;
    uint16_t maxPixelClockLow;
    uint16_t maxPixelClockHigh;
    unsigned char rsvb[190];
};

enum i686_vbe_constants {
    i686_vbe_PreserveContents = 1 << 15,
    i686_vbe_LFB = 1 << 14,
    i686_vbe_Mode_IsSupported = 1,
    i686_vbe_Mode_OptInfo = 1 << 1,
    i686_vbe_Mode_BiosOutputSupported = 1 << 2,
    i686_vbe_Mode_IsColor = 1 << 3,
    i686_vbe_Mode_IsGraphics = 1 << 4,
    i686_vbe_Mode_IsNorVGAComp = 1 << 5,
    i686_vbe_Mode_NoBanks = 1 << 6,
    i686_vbe_Mode_LFB = 1 << 7,
    i686_vbe_Mode_DoubleScan = 1 << 8,
    i686_vbe_Mode_InterlacedSupport = 1 << 9,
    i686_vbe_Mode_TriBufSupport = 1 << 10,
    i686_vbe_Mode_StereoSupport = 1 << 11,
    i686_vbe_Mode_DualDispStartAddr = 1 << 11,
};

#ifdef __cplusplus
extern "C" {
#endif

extern const unsigned char i686_vbe_rom[];

void i686_bios_video_WriteString(int mode, int pageNum, int color, size_t stringSize, int row, int column, const char *str);
i686_bios_video_ModeInfo i686_bios_video_GetVideoMode(void);
i686_bios_video_CursorInfo i686_bios_video_GetCursorPosShape(void);
void i686_bios_video_SetCursorShape(int startOpts, int end);
void i686_bios_video_SetCursorPos(int page, uint8_t row, uint8_t column);

void i686_vbe_GetInformation(i686_vbe_Info *buf);
void i686_vbe_GetModeInfo(uint16_t mode, i686_vbe_ModeInfo *buf);

#ifdef __cplusplus
} // extern "C"

namespace i686::bios::video {

enum WrStrMode {
    WrStrMode_UpdateCursor = i686_VideoBIOS_WrStrMode_UpdateCursor,
    WrStrMode_WithAttribute = i686_VideoBIOS_WrStrMode_WithAttribute
};
enum CursorMode {
    CursorMode_Normal = i686_VideoBIOS_CursorMode_Normal,
    CursorMode_Invisible = i686_VideoBIOS_CursorMode_Invisible,
    CursorMode_Mask = i686_VideoBIOS_CursorMode_Mask,
};
using VideoModeInfo = i686_bios_video_ModeInfo;
using CursorInfo = i686_bios_video_CursorInfo;

inline constexpr auto &WriteString = i686_bios_video_WriteString;
inline constexpr auto &GetVideoMode = i686_bios_video_GetVideoMode;
inline constexpr auto &GetCursorPosShape = i686_bios_video_GetCursorPosShape;
inline constexpr auto &SetCursorShape = i686_bios_video_SetCursorShape;
inline constexpr auto &SetCursorPos = i686_bios_video_SetCursorPos;

}

namespace i686::vbe {

using Info = i686_vbe_Info;
using ModeInfo = i686_vbe_ModeInfo;

enum constants {
    PreserveContents = i686_vbe_PreserveContents,
    LFB = i686_vbe_LFB,
};

enum Mode {
    Mode_IsSupported = i686_vbe_Mode_IsSupported,
    Mode_OptInfo = i686_vbe_Mode_OptInfo,
    Mode_BiosOutputSupported = i686_vbe_Mode_BiosOutputSupported,
    Mode_IsColor = i686_vbe_Mode_IsColor,
    Mode_IsGraphics = i686_vbe_Mode_IsGraphics,
    Mode_IsNorVGAComp = i686_vbe_Mode_IsNorVGAComp,
    Mode_NoBanks = i686_vbe_Mode_NoBanks,
    Mode_LFB = i686_vbe_Mode_LFB,
    Mode_DoubleScan = i686_vbe_Mode_DoubleScan,
    Mode_InterlacedSupport = i686_vbe_Mode_InterlacedSupport,
    Mode_TriBufSupport = i686_vbe_Mode_TriBufSupport,
    Mode_StereoSupport = i686_vbe_Mode_StereoSupport,
    Mode_DualDispStartAddr = i686_vbe_Mode_DualDispStartAddr,
};

inline constexpr auto &rom = i686_vbe_rom;

inline constexpr auto &GetInformation = i686_vbe_GetInformation;
inline constexpr auto &GetModeInfo = i686_vbe_GetModeInfo;

}

#endif

#endif // VIDEO_BIOS_H
