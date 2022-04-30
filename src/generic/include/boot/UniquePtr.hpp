#ifndef UNIQUEPTR_H
#define UNIQUEPTR_H

#include <type_traits>
#include <cstdint>

namespace boot {

template <typename T>
struct DefaultDeleter;

template <typename T, typename D = DefaultDeleter<T>>
class UniquePtr;

template <typename T>
struct DefaultDeleter {
    void operator()(T* ptr) {
        delete ptr;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {
    void operator()(T *ptr) const {
        delete[] ptr;
    }
};

namespace detail {

template <typename T>
class RemoveVLA {
public:
    using Type = T;
};

template <typename T>
class RemoveVLA<T[]> {
public:
    using Type = T;
};

template <typename T, typename D>
class UniquePtr_Base {
public:
    using ElementType = typename RemoveVLA<T>::Type;
    using Pointer = ElementType*;
    friend class UniquePtr<T, D>;
private:
    Pointer ptr;
public:
    constexpr UniquePtr_Base(Pointer ptr) : ptr(ptr) {}
    UniquePtr_Base(const UniquePtr_Base &src) = delete;
    UniquePtr_Base &operator=(const UniquePtr_Base &src) = delete;
    UniquePtr_Base(UniquePtr_Base &&src) :
        ptr(src.ptr)
    {
        src.ptr = nullptr;
    }
    UniquePtr_Base &operator=(UniquePtr_Base &&src)
    {
        ptr = src.ptr;
        src.ptr = nullptr;
    }
    ~UniquePtr_Base()
    {
        Reset();
    }
    void Reset(Pointer ptr = nullptr)
    {
        D del = {};
        del(this->ptr);
        this->ptr = ptr;
    }
    [[nodiscard]]
    auto Release() -> Pointer
    {
        auto r = ptr;
        ptr = nullptr;
        return r;
    }
    auto Get() const -> Pointer
    {
        return ptr;
    }
    operator bool() const
    {
        return ptr != nullptr;
    }
    auto operator*() const -> ElementType&
    {
        return *Get();
    }
    auto operator->() const -> Pointer
    {
        return Get();
    }
};

}

template <typename T, typename D>
class UniquePtr : public detail::UniquePtr_Base<T, D> {
    using Base = detail::UniquePtr_Base<T, D>;
    using ElementType = T;
    using Pointer = T*;
public:
    UniquePtr() : Base(nullptr) {}
    UniquePtr(Pointer ptr) : Base(ptr) {}
};

template <typename T, typename D>
class UniquePtr<T[], D> : public detail::UniquePtr_Base<T[], D> {
    using Base = detail::UniquePtr_Base<T[], D>;
    using ElementType = T;
    using Pointer = T*;
public:
    UniquePtr() : Base(nullptr) {}
    UniquePtr(Pointer ptr) : Base(ptr) {}
    T &operator[](std::size_t index) const
    {
        return this->Get()[index];
    }
};

}

#endif // UNIQUEPTR_H
