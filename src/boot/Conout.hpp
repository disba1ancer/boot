#ifndef BOOT_CONOUT_HPP
#define BOOT_CONOUT_HPP

#include "UniquePtr.h"

namespace boot {

class Conout_impl;

enum constants {
    boot_conout_FromBegin,
    boot_conout_FromCurrent
};

class Conout
{
public:
    enum From {
        FromScreenStart,
        FromCursor
    };
private:
    UniquePtr<Conout_impl> impl;
    Conout();
public:
    static auto Instance() -> Conout&;
    void PutC(char c);
    void Write(const char *buf, std::size_t size);
    void PutStr(const char *str);
    void Seek(std::size_t pos, From origin);
    auto Tell() -> std::size_t;
    auto LineLen() -> std::size_t;
    auto Lines() -> std::size_t;
};

} // namespace boot

#endif // BOOT_CONOUT_HPP
