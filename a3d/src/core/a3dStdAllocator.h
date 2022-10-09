//-------------------------------------------------------------------------------------------------
// File : a3dStdAllocator.h
// Desc : Std Allocator.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// StdAllocator class
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct StdAllocator
{
    using value_type = T;

    StdAllocator() 
    { /* DO_NOTHING */ }

    template<typename U>
    StdAllocator(const StdAllocator<U>&)
    { /* DO_NOTHING */ }

    T* allocate(std::size_t count)
    { return reinterpret_cast<T*>(a3d_alloc(sizeof(T) * count, alignof(T))); }

    void deallocate(T* ptr, std::size_t)
    { a3d_free(ptr); }
};

template<typename T, typename U>
bool operator == (const StdAllocator<T>&, const StdAllocator<U>&)
{ return true; }

template<typename T, typename U>
bool operator != (const StdAllocator<T>&, const StdAllocator<U>&)
{ return false; }

} // namespace a3d
