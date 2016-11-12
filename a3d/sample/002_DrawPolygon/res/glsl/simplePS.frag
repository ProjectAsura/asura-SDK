//-------------------------------------------------------------------------------------------------
// File : simplePS.frag
// Desc : Simple Fragment Shader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

//-------------------------------------------------------------------------------------------------
// Input Definitions.
//-------------------------------------------------------------------------------------------------
layout(location = 0) in vec4 InputColor;

//-------------------------------------------------------------------------------------------------
// Output Definitions.
//-------------------------------------------------------------------------------------------------
layout(location = 0) out vec4 ColorTarget0;


//-------------------------------------------------------------------------------------------------
//      フラグメントシェーダメインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
void main()
{
    ColorTarget0 = InputColor;
}