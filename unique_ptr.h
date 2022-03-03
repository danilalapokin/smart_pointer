#pragma once

#include "compressed_pair.h"
#include <cstddef>
#include <utility>
#include <memory>
#include <cstddef>

// Primary template
template <typename T, typename Deleter = std::default_delete<T>>
class UniquePtr {

public:
    CompressedPair<T*, Deleter> data_;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }

    template <typename T1, typename T2>
    UniquePtr(T1&& ptr, T2&& del) noexcept : data_(std::forward<T1>(ptr), std::forward<T2>(del)) {
    }

    template <typename T1, typename T2>
    UniquePtr(UniquePtr<T1, T2>&& other) noexcept
        : data_(std::forward<T1*>(std::exchange(other.data_.GetFirst(), nullptr)),
                std::forward<T2>(other.data_.GetSecond())) {
    }

    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr& operator=(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        data_.GetSecond()(data_.GetFirst());
        data_.GetFirst() = nullptr;
        std::swap(data_.GetFirst(), other.data_.GetFirst());
        data_.GetSecond() = std::forward<Deleter>(other.data_.GetSecond());
        return *this;
    };

    UniquePtr& operator=(std::nullptr_t) {
        data_.GetSecond()(data_.GetFirst());
        data_.GetFirst() = nullptr;
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (data_.GetFirst() != nullptr) {
            data_.GetSecond()(data_.GetFirst());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* temp = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return temp;
    }
    void Reset(T* ptr = nullptr) {
        auto temp = data_.GetFirst();
        data_.GetFirst() = ptr;
        if (temp != nullptr) {
            data_.GetSecond()(temp);
        }
    }
    void Swap(UniquePtr& other) noexcept {
        std::swap(data_.GetFirst(), other.data_.GetFirst());
        std::swap(data_.GetSecond(), other.data_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    };
    Deleter& GetDeleter() {
        return data_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return data_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const {
        return *(data_.GetFirst());
    }
    T* operator->() const noexcept {
        return data_.GetFirst();
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
private:
    using TT = typename std::remove_extent<T>::type;

public:
    void operator()(TT* ptr) {
        if (ptr != nullptr) {
            delete[] ptr;
        }
    }
};
