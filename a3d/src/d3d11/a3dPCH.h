//-------------------------------------------------------------------------------------------------
// File : a3dPCH.h
// Desc : Pre-Compile Header.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

#define A3D_FOR_WINDOWS10

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <allocator/a3dBaseAllocator.h>
#include <allocator/a3dStdAllocator.h>
#include <a3d.h>
#include <cassert>
#include <atomic>
#include <mutex>
#include <container/a3dDynamicArray.h>

#if defined(A3D_FOR_WINDOWS10)
#include <d3d11_4.h>
#include <dxgi1_5.h>
#else
#include <d3d11_2.h>
#include <dxgi1_3.h>
#endif

#include "emu/a3dImCmd.h"
#include "emu/a3dCommandBuffer.h"
#include "emu/a3dCommandList.h"

#include "misc/a3dBlob.h"

#include "a3dUtil.h"
#include "a3dDevice.h"
#include "a3dFence.h"
#include "a3dCommandSet.h"
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

#ifndef A3D_ASSERT
    #if defined(DEBUG) || defined(_DEBUG)
        #define     A3D_ASSERT(expression)  assert(expression)
    #else
        #define     A3D_ASSERT(expression)
    #endif
#endif

