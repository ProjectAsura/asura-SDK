//-------------------------------------------------------------------------------------------------
// File : a3dList.h
// Desc : List Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <list>
#include <allocator/a3dStdAllocator.h>


namespace a3d {

template<typename T>
using list = std::list<T, StdAllocator<T>>;

} // namespace a3d
