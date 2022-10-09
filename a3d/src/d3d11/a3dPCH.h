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
#include <cassert>
#include <atomic>

#include "core/a3dBaseAllocator.h"
#include "core/a3dStdAllocator.h"
#include "core/a3dLogger.h"
#include "core/a3dSpinLock.h"

#include <a3d.h>

#if defined(A3D_FOR_WINDOWS10)
#include <d3d11_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
using ID3D11DeviceA3D           = ID3D11Device5;
using ID3D11DeviceContextA3D    = ID3D11DeviceContext4;
#else
#include <d3d11_2.h>
#include <dxgi1_3.h>
using ID3D11DeviceA3D           = ID3D11Device2;
using ID3D11DeviceContextA3D    = ID3D11DeviceContext2;
#endif

#include "emu/a3dImCmd.h"
#include "emu/a3dCommandBuffer.h"
#include "emu/a3dCommandList.h"


#include "a3dUtil.h"
#include "a3dDevice.h"
#include "a3dFence.h"
#include "a3dCommandSet.h"
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

#ifndef A3D_ASSERT
    #if defined(DEBUG) || defined(_DEBUG)
        #define     A3D_ASSERT(expression)  assert(expression)
    #else
        #define     A3D_ASSERT(expression)
    #endif
#endif

