#ifndef BOOT_CONOUT_IMPL_H
#define BOOT_CONOUT_IMPL_H

#include "boot/Conout.hpp"
#include "bios_video.h"

namespace boot {

class Conout_impl
{
public:
    using From = Conout::From;
private:
    using VideoModeInfo = i686::VideoBIOS::VideoModeInfo;
    static constexpr std::size_t rows = 25;
    VideoModeInfo modeInfo;
    std::size_t bufferSize;
    UniquePtr<std::uint16_t[]> buffer;
    std::size_t startRow;
    std::size_t cursor;
    void Render();
    void PutC_intern(char c);
public:
    Conout_impl();
    void PutC(char c);
    void Write(const char *buf, std::size_t size);
    void PutStr(const char *str);
    void Seek(std::size_t pos, From origin);
    auto Tell() -> std::size_t;
    auto LineLen() -> std::size_t;
    auto Lines() -> std::size_t;
};

} // namespace boot

#endif // BOOT_CONOUT_IMPL_H
