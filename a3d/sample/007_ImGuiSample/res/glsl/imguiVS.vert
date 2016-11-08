//-------------------------------------------------------------------------------------------------
// File : simpleVS.vert
// Desc : Simple Vertex Shader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

//-------------------------------------------------------------------------------------------------
// Input Definitions.
//-------------------------------------------------------------------------------------------------
layout(location = 0) in vec2 InputPosition; // 位置座標.
layout(location = 1) in vec2 InputTexCoord; // テクスチャ座標.
layout(location = 2) in vec4 InputColor;    // カラー


//-------------------------------------------------------------------------------------------------
// Output Definitions.
//-------------------------------------------------------------------------------------------------
layout(location = 0) out vec4 OutputColor;     // カラー
layout(location = 1) out vec2 OutputTexCoord;  // テクスチャ座標.

out gl_PerVertex
{
    vec4 gl_Position;   // 位置座標.
};

//-------------------------------------------------------------------------------------------------
// Uniform Buffers
//-------------------------------------------------------------------------------------------------
layout(binding = 0) uniform Transform
{
    mat4    Proj;       // 射影行列.
} transform;

//-------------------------------------------------------------------------------------------------
//      頂点シェーダメインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
void main()
{
    gl_Position    = transform.Proj * vec4(InputPosition, 0.0f, 1.0f);
    OutputColor    = InputColor;
    OutputTexCoord = InputTexCoord;

    // Flip Y-Coordinate.
    gl_Position.y = -gl_Position.y;
}