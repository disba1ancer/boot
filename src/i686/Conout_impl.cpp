#include "Conout_impl.h"
#include "vgacon.h"
#include <stdlib.h>
#include <new>

namespace boot {

void Conout_impl::Render()
{
    auto vga_begin = i686::vga::console;
    const auto row_begin = buffer.Get() + startRow * modeInfo.cols;
    const auto buf_end = buffer.Get() + bufferSize;
    auto current = row_begin;
    while (current != buf_end) {
        *vga_begin++ = *current++;
    }
    current = buffer.Get();
    while (current != row_begin) {
        *vga_begin++ = *current++;
    }
    auto col = cursor % modeInfo.cols;
    auto row = cursor / modeInfo.cols;
    auto newRow = row - startRow;
    newRow += rows * (newRow > row);
    i686::bios::video::SetCursorPos(modeInfo.page, newRow, col);
}

void Conout_impl::PutC_intern(char c)
{
    switch (c) {
        case '\n':
            cursor += modeInfo.cols;
            cursor -= cursor % modeInfo.cols;
            break;
        case '\r':
            cursor -= cursor % modeInfo.cols;
            return;
        case '\b':
            if (cursor != startRow * modeInfo.cols) {
                if (cursor == 0) {
                    cursor = bufferSize - 1;
                } else {
                    --cursor;
                }
            }
            buffer[cursor] = 0x700;
            return;
        default:
            buffer[cursor++] = 0x700 + ((unsigned char)c);
    }
    cursor %= bufferSize;
    if (cursor == startRow * modeInfo.cols) {
        std::size_t i = startRow * modeInfo.cols;
        std::size_t end = i + modeInfo.cols;
        for (; i < end; ++i) {
            buffer[i] = 0x700;
        }
        startRow = (startRow + 1) % rows;
    }
}

Conout_impl::Conout_impl() :
    modeInfo(i686::bios::video::GetVideoMode()),
    bufferSize(modeInfo.cols * rows),
    buffer(new uint16_t[bufferSize]),
    startRow(0),
    cursor(0)
{
    if (!buffer) {
        abort();
    }
    auto end = buffer.Get() + bufferSize;
    for(auto i = buffer.Get(); i != end; ++i) {
        *i = 0x700;
    }
    Render();
}

void Conout_impl::PutC(char c)
{
    if (c == 0) return;
    PutC_intern(c);
    Render();
}

void Conout_impl::Write(const char* buf, std::size_t size)
{
    for (std::size_t i = 0; i < size; ++i) {
        PutC_intern(buf[i]);
    }
    Render();
}

void Conout_impl::PutStr(const char* str)
{
    while (*str) {
        PutC_intern(*str++);
    }
    Render();
}

void Conout_impl::Seek(std::size_t pos, From origin)
{
    switch (origin) {
        case From::FromCursor: {
            cursor = (cursor + pos) % (rows * modeInfo.cols);
        }
        case From::FromScreenStart: {

        }
    }
}

} // namespace boot
