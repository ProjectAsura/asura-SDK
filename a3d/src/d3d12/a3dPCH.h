//-------------------------------------------------------------------------------------------------
// File : a3dPCH.h
// Desc : Pre-Compile Header.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


#include <cassert>

#ifndef A3D_ASSERT
    #if defined(DEBUG) || defined(_DEBUG)
        #define     A3D_ASSERT(expression)  assert(expression)
    #else
        #define     A3D_ASSERT(expression)
    #endif
#endif

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <allocator/a3dBaseAllocator.h>
#include <allocator/a3dStdAllocator.h>
#include <a3d.h>

#include <atomic>
#include <mutex>
#include <container/a3dDynamicArray.h>
#include <container/a3dPool.h>

#include <d3d12.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>

#include "misc/a3dBlob.h"

#include "a3dUtil.h"
#include "a3dDescriptor.h"
#include "a3dDescriptorHeap.h"
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
#include "a3dStorageView.h"
#include "a3dSampler.h"
#include "a3dFrameBuffer.h"
#include "a3dDescriptorSetLayout.h"
#include "a3dDescriptorSet.h"
#include "a3dPipelineState.h"
#include "a3dQueryPool.h"


