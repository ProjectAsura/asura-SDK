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
layout(location = 0) in vec3 InputPosition; // 位置座標.
layout(location = 1) in vec2 InputTexCoord; // テクスチャ座標.

//-------------------------------------------------------------------------------------------------
// Output Definitions.
//-------------------------------------------------------------------------------------------------
layout(location = 0) out vec2 OutputTexCoord;  // テクスチャ座標.

out gl_PerVertex
{
    vec4 gl_Position;   // 位置座標.
};

//-------------------------------------------------------------------------------------------------
// Uniform Buffers
//-------------------------------------------------------------------------------------------------
layout(binding = 0) uniform Transform
{
    mat4    World;      // ワールド（モデル）行列.
    mat4    View;       // ビュー行列.
    mat4    Proj;       // 射影行列.
} transform;

//-------------------------------------------------------------------------------------------------
//      頂点シェーダメインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
void main()
{
    vec4 localPos = vec4(InputPosition, 1.0f);

    vec4 worldPos = transform.World * localPos;
    vec4 viewPos  = transform.View  * worldPos;
    vec4 projPos  = transform.Proj  * viewPos;

    gl_Position = projPos;
    OutputTexCoord = InputTexCoord;

    // Flip Y-Coordinate.
    gl_Position.y = -gl_Position.y;
}