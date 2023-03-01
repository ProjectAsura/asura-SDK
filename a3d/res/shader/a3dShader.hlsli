//-------------------------------------------------------------------------------------------------
// File : a3dShader.hlsli
// Desc : Shader Helper.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#ifndef A3D_SHADER_HLSLI
#define A3D_SHADER_HLSLI

#ifdef __spirv__
#define A3D_LOCATION(idx)   [[vk::location(idx)]]
#define A3D_BINDING(idx)    [[vk::binding(idx)]]
#define A3D_COMBINED        [[vk::combinedImageSampler]]
#define A3D_PUSH_CONSTANT   [[vk::push_constant]]
#define A3D_FLIP_Y(var)     var.y = -var.y
#else
#define A3D_LOCATION(idx)
#define A3D_BINDING(idx)
#define A3D_COMBINED
#define A3D_PUSH_CONSTANT
#define A3D_FLIP_Y(var)
#endif

#ifdef __PSSL__
#include "a3dPSSL.hlsli"
#endif

#endif//A3D_SHADER_HLSLI
