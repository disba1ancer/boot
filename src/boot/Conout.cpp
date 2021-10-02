#include "Conout.hpp"
#include <new>
#include "i686/Conout_impl.h"

namespace boot {

Conout::Conout() :
    impl(new(std::nothrow) Conout_impl())
{}

auto Conout::Instance() -> Conout&
{
    static Conout instance = {};
    return instance;
}

void Conout::PutC(char c)
{
    impl->PutC(c);
}

void Conout::Write(const char* buf, std::size_t size)
{
    impl->Write(buf, size);
}

void Conout::PutStr(const char* str)
{
    impl->PutStr(str);
}

void Conout::Seek(std::size_t pos, From origin)
{
    impl->Seek(pos, origin);
}

std::size_t Conout::Tell()
{}

std::size_t Conout::LineLen()
{}

std::size_t Conout::Lines()
{}

} // namespace boot
