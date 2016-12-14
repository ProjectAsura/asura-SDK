//-------------------------------------------------------------------------------------------------
// File : a3dDynamicArray.h
// Desc : Dynamic Array Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <vector>
#include <allocator/a3dStdAllocator.h>


namespace a3d {

template<typename T>
using dynamic_array = std::vector<T, StdAllocator<T>>;

} // namespace a3d
