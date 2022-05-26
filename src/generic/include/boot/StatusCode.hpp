#ifndef STATUSCODE_HPP
#define STATUSCODE_HPP

#include <stdint.h>
#include <concepts>

namespace boot {

enum class StatusCode : uint32_t;

bool IsErrorCode(const StatusCode& status)
{
    return uint32_t(status) & 0x80000000;
}

int GetCodeClass(const StatusCode& status)
{
    return (uint32_t(status) & 0x7FFF0000) >> 16;
}

uint32_t GetCodeValue(const StatusCode& status)
{
    return (uint32_t(status) & 0x8000FFFFU);
}

template <typename T>
T GetCodeValue(const StatusCode& status)
{
    return T(GetCodeValue(status));
}

StatusCode ConstructStatusCode(int cls, uint32_t code)
{
    return StatusCode(((uint32_t(cls) & 0x7FFFU) << 16) | (code & 0x8000FFFFU));
}

template <typename T>
StatusCode ConstructStatusCode(int cls, T code)
{
    return ConstructStatusCode(cls, uint32_t(code));
}

}

#endif // STATUSCODE_H
