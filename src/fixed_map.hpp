#pragma once
#ifndef __FIXED_MAP_HPP__
#define __FIXED_MAP_HPP__

#include <string.h>
#include <macros.hpp>

//TODO version with NONE elem as the one that is invalid

//For enums
template <typename T>
concept IsEnum = std::is_enum_v<T>;
template <typename T>
concept IsNotEnum = not IsEnum<T>;

template <IsEnum EnumType>
constexpr typename std::underlying_type<EnumType>::type to_underlying(EnumType e) noexcept requires IsEnum<EnumType> {
    return static_cast<typename std::underlying_type<EnumType>::type>(e);
}

template <IsNotEnum NonEnumType>
constexpr NonEnumType to_underlying(NonEnumType e) noexcept requires IsNotEnum<NonEnumType> {
    return (e);
}

//NONE element is the one that is invalid
template <auto EnumSize, typename Type>
class FixedMap {
    //basically exists to allow enum size
    static constexpr auto Size = to_underlying(EnumSize);

public:
    FixedMap() {
        clear();
    }

    Type& operator[](auto index) {
        validate_bounds(to_underlying(index), Size);
        presence[to_underlying(index)] = true;
        return data[to_underlying(index)];
    }

    const Type& operator[](auto index) const {
        validate_bounds(to_underlying(index), Size);
        assert(presence[to_underlying(index)] && "accessing non-presented element");
        return data[to_underlying(index)];
    }

    void insert(unsigned int index, const Type& value) {
        validate_bounds(index, Size);
        data[index] = value;
        presence[index] = true;
    }

    bool contains(unsigned int index) const {
        validate_bounds(index, Size);
        return presence[index];
    }

    Type* find(unsigned int index) {
        validate_bounds(index, Size);
        return presence[index] ? &data[index] : nullptr;
    }

    const Type* find(unsigned int index) const {
        validate_bounds(index, Size);
        return presence[index] ? &data[index] : nullptr;
    }

    void clear() {
        memset(data, 0, sizeof(data));
        memset(presence, 0, sizeof(presence));
    }

    class iterator {
    public:
        iterator(Type* ptr) : m_ptr(ptr) {}
        
        Type& operator*() const {
            return *m_ptr;
        }
        
        iterator& operator++() {
            m_ptr++;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            (*this)++;
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return m_ptr == other.m_ptr;
        }

        bool operator!=(const iterator& other) const {
            return m_ptr != other.m_ptr;
        }

    private:
        Type* m_ptr;
    };

    iterator begin() {
        return iterator(data);
    }
    iterator end() {
        return iterator(data + Size);
    }

private:
    Type data[Size] = {};
    bool presence[Size] = {};
};
#endif // __FIXED_MAP_HPP__