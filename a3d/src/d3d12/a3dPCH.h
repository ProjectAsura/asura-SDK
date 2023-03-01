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
#include <atomic>

#include "core/a3dBaseAllocator.h"
#include "core/a3dStdAllocator.h"
#include "core/a3dSpinLock.h"
#include "core/a3dList.h"
#include "core/a3dLogger.h"
#include "core/a3dFixString.h"

#include <a3d.h>

#if A3D_IS_WIN
#include <d3d12.h>
#include <dxgi1_6.h>

using IDXGIFactoryA3D   = IDXGIFactory5;
using IDXGIAdapterA3D   = IDXGIAdapter4;
using IDXGIOutputA3D    = IDXGIOutput6;
using IDXGISwapChainA3D = IDXGISwapChain4;
using ID3D12DeviceA3D   = ID3D12Device8;
#else
#include "a3dNative.h"
#endif

#include <D3D12MemAlloc.h>

#include "a3dUtil.h"
#include "a3dDescriptor.h"
#include "a3dDescriptorHeap.h"
#include "a3dDevice.h"
#include "a3dFence.h"
#include "a3dCommandList.h"
#include "a3dQueue.h"
#include "a3dSwapChain.h"
#include "a3dBuffer.h"
#include "a3dConstantBufferView.h"
#include "a3dTexture.h"
#include "a3dRenderTargetView.h"
#include "a3dDepthStencilView.h"
#include "a3dShaderResourceView.h"
#include "a3dUnorderedAccessView.h"
#include "a3dSampler.h"
#include "a3dDescriptorSetLayout.h"
#include "a3dPipelineState.h"
#include "a3dQueryPool.h"
#include "a3dDred.h"
#include "a3dAccelerationStructure.h"
#include "a3dRayTracingPipelineState.h"


