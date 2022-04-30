#ifndef BIOS_DISK_H
#define BIOS_DISK_H

#include <stdint.h>
#include "boot/util.h"
#include "processor.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOT_STRUCT(i686_bios_disk_DriveParameters1) {
    uint16_t size;
    uint16_t flags;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectPerTrack;
    uint64_t sectorsTotal;
    uint16_t bytesPerSect;
};

#define i686_bios_disk_DriveParameters1Size 0x1A

BOOT_STRUCT(i686_bios_disk_DriveParameters2) {
    uint16_t size;
    uint16_t flags;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectPerTrack;
    uint64_t sectorsTotal;
    uint16_t bytesPerSect;
    i686_RMPtr eddCfgParams;
};

#define i686_bios_disk_DriveParameters2Size 0x1E

BOOT_STRUCT(i686_bios_disk_InterfacePathISA) {
    uint16_t baseAddress;
};

BOOT_STRUCT(i686_bios_disk_InterfacePathPCI) {
    uint8_t busNum;
    uint8_t devNum;
    uint8_t funcNum;
};

BOOT_STRUCT(i686_bios_disk_DevicePathATA) {
    uint8_t isSlave;
};

BOOT_STRUCT(i686_bios_disk_DevicePathATAPI) {
    uint8_t isSlave;
    uint8_t logicalUnitNum;
};

BOOT_STRUCT(i686_bios_disk_DevicePathSCSI) {
    uint8_t logicalUnitNum;
};

BOOT_STRUCT(i686_bios_disk_DevicePathUSB) {
    uint8_t unk;
};

BOOT_STRUCT(i686_bios_disk_DevicePathIEEE1394) {
    uint64_t guid;
};

BOOT_STRUCT(i686_bios_disk_DevicePathFibreChanel) {
    uint64_t worldWideNumber;
};

BOOT_STRUCT(i686_bios_disk_DriveParameters3) {
    uint16_t size;
    uint16_t flags;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectPerTrack;
    uint64_t sectorsTotal;
    uint16_t bytesPerSect;
    i686_RMPtr eddCfgParams;
    uint16_t magic;
    uint8_t pathDataLen;
    char rsv0[3];
    char hstBus[4];
    char interfaceType[8];
    union {
        i686_bios_disk_InterfacePathISA isa;
        i686_bios_disk_InterfacePathPCI pci;
        unsigned char rsv[8];
    } interfacePath;
    union {
        i686_bios_disk_DevicePathATA ata;
        i686_bios_disk_DevicePathATAPI atapi;
        i686_bios_disk_DevicePathSCSI scsi;
        i686_bios_disk_DevicePathUSB usb;
        i686_bios_disk_DevicePathIEEE1394 ieee1394;
        i686_bios_disk_DevicePathFibreChanel fibre;
        unsigned char rsv[16];
    } devicePath;
    char rsv1;
    unsigned char checksum;
};

#define i686_bios_disk_DriveParameters3Size 0x4A

BOOT_STRUCT(i686_bios_disk_AddressPacket) {
    uint8_t size;
    unsigned char rsv0;
    uint8_t blkCount;
    unsigned char rsv1;
    i686_RMPtr buffer;
    alignas(8) uint64_t lba;
};

int i686_bios_disk_GetDriveParameters(int diskNum, void *buffer);
int i686_bios_disk_Read(int diskNum, i686_bios_disk_AddressPacket *addrPkt);

#ifdef __cplusplus
}

namespace i686::bios::disk {

using DriveParameters1 = ::i686_bios_disk_DriveParameters1;
using DriveParameters2 = ::i686_bios_disk_DriveParameters2;
using DriveParameters3 = ::i686_bios_disk_DriveParameters3;
using InterfacePathISA = ::i686_bios_disk_InterfacePathISA;
using InterfacePathPCI = ::i686_bios_disk_InterfacePathPCI;
using DevicePathATA = ::i686_bios_disk_DevicePathATA;
using DevicePathATAPI = ::i686_bios_disk_DevicePathATAPI;
using DevicePathSCSI = ::i686_bios_disk_DevicePathSCSI;
using DevicePathUSB = ::i686_bios_disk_DevicePathUSB;
using DevicePathIEEE1394 = ::i686_bios_disk_DevicePathIEEE1394;
using DevicePathFibreChanel = ::i686_bios_disk_DevicePathFibreChanel;

inline constexpr uint16_t DriveParameters1Size = i686_bios_disk_DriveParameters1Size;
#undef i686_bios_disk_DriveParameters1Size
inline constexpr uint16_t DriveParameters2Size = i686_bios_disk_DriveParameters2Size;
#undef i686_bios_disk_DriveParameters2Size
inline constexpr uint16_t DriveParameters3Size = i686_bios_disk_DriveParameters3Size;
#undef i686_bios_disk_DriveParameters3Size

using AddressPacket = i686_bios_disk_AddressPacket;

inline constexpr auto& GetDriveParameters = ::i686_bios_disk_GetDriveParameters;
inline constexpr auto& Read = ::i686_bios_disk_Read;

}

#endif

#endif // BIOS_DISK_H
