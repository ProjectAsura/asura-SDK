//-------------------------------------------------------------------------------------------------
// File : a3dNullHandle.h
// Desc : Null Handle Definitions.
// Copyright(c) Project Asura. All right reserved
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>


///////////////////////////////////////////////////////////////////////////////////////////////////
// NullHandle class
///////////////////////////////////////////////////////////////////////////////////////////////////
const class NullHandle
{
public:
    template<typename T>
    operator T* () const
    { return nullptr; }

    template<typename S, typename T>
    operator T S::*() const
    { return 0; }

    operator uint64_t () const
    { return 0; }

private:
    void operator& () const = delete;
} null_handle = {};

