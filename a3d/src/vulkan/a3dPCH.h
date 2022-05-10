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
#include <cfloat>
#include <atomic>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "misc/a3dBlob.h"
#include "misc/a3dInlines.h"
#include "misc/a3dNullHandle.h"
#include "misc/a3dLogger.h"
#include "misc/a3dSpinLock.h"

#include "a3dDevice.h"
#include "a3dFence.h"
#include "a3dCommandSet.h"
#include "a3dCommandList.h"
#include "a3dQueue.h"
#include "a3dSwapChain.h"
#include "a3dBuffer.h"
#include "a3dTexture.h"
#include "a3dConstantBufferView.h"
#include "a3dRenderTargetView.h"
#include "a3dDepthStencilView.h"
#include "a3dShaderResourceView.h"
#include "a3dUnorderedAccessView.h"
#include "a3dSampler.h"
#include "a3dDescriptorSetLayout.h"
#include "a3dPipelineState.h"
#include "a3dQueryPool.h"
#include "a3dUtil.h"
#include "a3dSpirv.h"
#include "a3dAccelerationStructure.h"


#ifndef A3D_ASSERT
    #if defined(DEBUG) || defined(_DEBUG)
        #define     A3D_ASSERT(expression)  assert(expression)
    #else
        #define     A3D_ASSERT(expression)
    #endif
#endif
