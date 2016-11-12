//-------------------------------------------------------------------------------------------------
// File : a3dBaseAllocator.h
// Desc : Base Allocator.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>


namespace a3d {

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
constexpr size_t DefaultAlignment = 4;


//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
void* a3d_alloc  (uint64_t size, uint64_t alignment);
void* a3d_realloc(void* ptr, uint64_t size);
void  a3d_free   (void* ptr);

void  a3d_disable_counter();
void  a3d_enable_counter ();


///////////////////////////////////////////////////////////////////////////////////////////////////
// BaseAllocator class
///////////////////////////////////////////////////////////////////////////////////////////////////
class BaseAllocator
{
public:
    static void* operator new (size_t size) noexcept
    { return a3d_alloc(size, DefaultAlignment); }

    static void* operator new[] (size_t size) noexcept
    { return a3d_alloc(size, DefaultAlignment); }

    static void* operator new (size_t size, void* memory) noexcept
    {
        (void)size;
        return memory;
    }

    static void* operator new[] (size_t size, void* memory) noexcept
    {
        (void)size;
        return memory;
    }

    static void operator delete (void* ptr) noexcept
    { a3d_free(ptr); }

    static void operator delete[] (void* ptr) noexcept
    { a3d_free(ptr); }

    static void operator delete (void*, void*) noexcept
    { /* DO_NOTHING */ }

    static void operator delete[] (void*, void*) noexcept
    { /* DO_NOTHING */ }
};

} // namespace a3d
