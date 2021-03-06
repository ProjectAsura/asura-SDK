﻿//-------------------------------------------------------------------------------------------------
// File : a3dBaseAllocator.cpp
// Desc : Base Allocator.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------------------------------
a3d::IAllocator*        g_pAllocator    = nullptr;
std::atomic<uint64_t>   g_AllocCounter  = {};
std::atomic<bool>       g_CounterEnable = {};

} // namespace /* anonymous */


//-------------------------------------------------------------------------------------------------
//      メモリを確保します.
//-------------------------------------------------------------------------------------------------
void* a3d_alloc(size_t size, size_t alignment )
{
    if (g_pAllocator == nullptr)
    { return nullptr; }

    auto ret = g_pAllocator->Alloc(size, alignment);

    if (g_CounterEnable)
    {
        if (ret != nullptr)
        { g_AllocCounter++; }
    }

    return ret;
}

//-------------------------------------------------------------------------------------------------
//      メモリを再確保します.
//-------------------------------------------------------------------------------------------------
void* a3d_realloc(void* ptr, size_t size, size_t alignment)
{
    if (g_pAllocator == nullptr)
    { return nullptr; }

    if (g_CounterEnable)
    {
        if (ptr == nullptr)
        { g_AllocCounter++; }
    }

    return g_pAllocator->Realloc(ptr, size, alignment);
}

//-------------------------------------------------------------------------------------------------
//      メモリを解放します.
//-------------------------------------------------------------------------------------------------
void a3d_free(void* ptr)
{
    if (g_pAllocator == nullptr)
    { return; }

    if (ptr != nullptr)
    {
        g_pAllocator->Free(ptr);

        if (g_CounterEnable)
        { g_AllocCounter--; }
    }
}

//-------------------------------------------------------------------------------------------------
//      カウンターを有効化します.
//-------------------------------------------------------------------------------------------------
void a3d_enable_counter(bool value)
{ g_CounterEnable = value; }


namespace a3d {

//-------------------------------------------------------------------------------------------------
//      システムアロケータを設定します.
//-------------------------------------------------------------------------------------------------
bool InitSystemAllocator(IAllocator* pAllocator)
{
    if ( g_pAllocator != nullptr )
    { return false; }

    g_pAllocator    = pAllocator;
    g_AllocCounter  = 0;
    g_CounterEnable = true;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      システムアロケータを未設定にします.
//-------------------------------------------------------------------------------------------------
void TermSystemAllocator()
{
    A3D_ASSERT( g_AllocCounter == 0 );
    g_pAllocator    = nullptr;
    g_AllocCounter  = 0;
    g_CounterEnable = false;
}

//-------------------------------------------------------------------------------------------------
//      システムアロケータが設定済みかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsInitSystemAllocator()
{ return g_pAllocator != nullptr; }

} // namespace a3d
