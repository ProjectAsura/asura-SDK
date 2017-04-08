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
#include <new>


//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
void* a3d_alloc(size_t size, size_t alignment);
void* a3d_realloc(void* ptr, size_t size, size_t alignment);
void  a3d_free(void* ptr);
void  a3d_enable_counter(bool enable);


namespace a3d {

//-------------------------------------------------------------------------------------------------
// Forward Declarations.
//-------------------------------------------------------------------------------------------------
struct IAllocator;

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
constexpr size_t DefaultAlignment = 4;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
bool InitSystemAllocator(IAllocator* pAllocator);
void TermSystemAllocator();
bool IsInitSystemAllocator();


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

    static void* operator new (size_t, void* memory) noexcept
    { return memory; }

    static void* operator new[] (size_t, void* memory) noexcept
    { return memory; }

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
