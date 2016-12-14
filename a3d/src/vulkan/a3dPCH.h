//-------------------------------------------------------------------------------------------------
// File : a3dPCH.h
// Desc : Pre-Compile Header.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <allocator/a3dBaseAllocator.h>
#include <allocator/a3dStdAllocator.h>
#include <a3d.h>
#include <cassert>
#include <atomic>
#include <mutex>

#include <vulkan/vulkan.h>

#include "a3dBlob.h"
#include "a3dDevice.h"
#include "a3dFence.h"
#include "a3dCommandSet.h"
#include "a3dCommandList.h"
#include "a3dQueue.h"
#include "a3dSwapChain.h"
#include "a3dBuffer.h"
#include "a3dBufferView.h"
#include "a3dTexture.h"
#include "a3dTextureView.h"
#include "a3dSampler.h"
#include "a3dFrameBuffer.h"
#include "a3dDescriptorSetLayout.h"
#include "a3dDescriptorSet.h"
#include "a3dPipelineState.h"
#include "a3dQueryPool.h"
#include "a3dUtil.h"

#ifndef A3D_ASSERT
    #if defined(DEBUG) || defined(_DEBUG)
        #define     A3D_ASSERT(expression)  assert(expression)
    #else
        #define     A3D_ASSERT(expression)
    #endif
#endif

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


template<typename T>
void SafeRelease(T*& ptr)
{
    if (ptr != nullptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}
