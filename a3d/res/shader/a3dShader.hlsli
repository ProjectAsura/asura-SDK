//-------------------------------------------------------------------------------------------------
// File : a3dShader.hlsli
// Desc : Shader Helper.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#ifndef A3D_SHADER_HLSLI
#define A3D_SHADER_HLSLI

#ifdef __spirv__
#define A3D_LOCATION(x)              [[vk::location(x)]]
#define A3D_RESOURCE(var, reg, x)    [[vk::binding(x)]] var
#define A3D_FLIP_Y(var)              var.y = -var.y
#else
#define A3D_LOCATION(x)
#define A3D_RESOURCE(var, reg, x)    var : register(reg)
#define A3D_FLIP_Y(var)
#endif

#endif//A3D_SHADER_HLSLI
