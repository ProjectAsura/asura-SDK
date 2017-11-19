//-------------------------------------------------------------------------------------------------
// File : a3dx_mesh.cpp
// Desc : Mesh Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3dx_resmesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cassert>
#include <codecvt>
#include <Allocator.h>


namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MeshLoader class
///////////////////////////////////////////////////////////////////////////////////////////////////
class MeshLoader
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    MeshLoader();
    ~MeshLoader();

    bool Load(const char* filename, a3d::Array<a3d::ResMesh>& meshes);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    const aiScene*      m_pScene;   // シーンデータ.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    void ParseMesh(a3d::ResMesh& dstMesh, const aiMesh* pSrcMesh);
};


//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
MeshLoader::MeshLoader()
: m_pScene(nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
MeshLoader::~MeshLoader()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      メッシュをロードします.
//-------------------------------------------------------------------------------------------------
bool MeshLoader::Load(const char* path, a3d::Array<a3d::ResMesh>& meshes)
{
    if (path == nullptr)
    { return false; }

    Assimp::Importer importer;
    int flag = 0;
    flag |= aiProcess_Triangulate;
    flag |= aiProcess_PreTransformVertices;
    flag |= aiProcess_CalcTangentSpace;
    flag |= aiProcess_GenSmoothNormals;
    flag |= aiProcess_GenUVCoords;
    flag |= aiProcess_RemoveRedundantMaterials;
    flag |= aiProcess_OptimizeMeshes;

    // ファイルを読み込み.
    m_pScene = importer.ReadFile(path, flag);

    // チェック.
    if (m_pScene == nullptr)
    { return false; }

    // メッシュのメモリを確保.
    meshes.term();
    meshes.init(Allocator::Instance(), m_pScene->mNumMeshes);

    // メッシュデータを変換.
    for(size_t i=0; i<meshes.size(); ++i)
    {
        const aiMesh* pMesh = m_pScene->mMeshes[i];
        ParseMesh(meshes[int(i)], pMesh);
    }

#if 0
    //// マテリアルのメモリを確保.
    //materials.clear();
    //materials.resize(m_pScene->mNumMaterials);
    //materials.shrink_to_fit();

    //// マテリアルデータを変換.
    //for(size_t i=0; i<materials.size(); ++i)
    //{
    //    const aiMaterial* pMaterial = m_pScene->mMaterials[i];
    //    ParseMaterial(materials[i], pMaterial);
    //}
#endif

    // 不要になったのでクリア.
    m_pScene = nullptr;

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      メッシュデータを解析します.
//-------------------------------------------------------------------------------------------------
void MeshLoader::ParseMesh(a3d::ResMesh& dstMesh, const aiMesh* pSrcMesh)
{
    // マテリアル番号を設定.
    auto mat_id = pSrcMesh->mMaterialIndex;
    auto mat = m_pScene->mMaterials[mat_id];

    aiString name;
    mat->Get(AI_MATKEY_NAME, name);
    dstMesh.MaterialTag = name.C_Str();

    aiVector3D zero3D(0.0f, 0.0f, 0.0f);

    // 頂点データのメモリを確保.
    //dstMesh.Vertices.resize(pSrcMesh->mNumVertices);
    auto vertex_count = pSrcMesh->mNumVertices;
    dstMesh.Positions.init(Allocator::Instance(), vertex_count);
    dstMesh.Normals  .init(Allocator::Instance(), vertex_count);
    dstMesh.Tangents .init(Allocator::Instance(), vertex_count);
    dstMesh.TexCoords.init(Allocator::Instance(), vertex_count);

    for(auto i=0u; i<vertex_count; ++i)
    {
        auto pPosition = &(pSrcMesh->mVertices[i]);
        auto pNormal   = &(pSrcMesh->mNormals[i]);
        auto pTexCoord = (pSrcMesh->HasTextureCoords(0)) ? &(pSrcMesh->mTextureCoords[0][i]) : &zero3D;
        auto pTangent  = (pSrcMesh->HasTangentsAndBitangents()) ? &(pSrcMesh->mTangents[i]) : &zero3D;

        dstMesh.Positions[i] = a3d::Vector3(pPosition->x, pPosition->y, pPosition->z);
        dstMesh.Normals  [i] = a3d::Vector3(pNormal  ->x, pNormal  ->y, pNormal  ->z);
        dstMesh.Tangents [i] = a3d::Vector3(pTangent ->x, pTangent ->y, pTangent ->z);
        dstMesh.TexCoords[i] = a3d::Vector2(pTexCoord->x, pTexCoord->y);
    }

    // 頂点インデックスのメモリを確保.
    dstMesh.Indices.init(Allocator::Instance(), pSrcMesh->mNumFaces * 3);

    for(auto i=0u; i<pSrcMesh->mNumFaces; ++i)
    {
        const auto& face = pSrcMesh->mFaces[i];
        assert(face.mNumIndices == 3);  // 三角形化しているので必ず3になっている.

        dstMesh.Indices[i * 3 + 0] = face.mIndices[0];
        dstMesh.Indices[i * 3 + 1] = face.mIndices[1];
        dstMesh.Indices[i * 3 + 2] = face.mIndices[2];
    }
}

} // namespace 


namespace a3dx {

bool CreateResMeshFromFile(const char* path, a3d::Array<a3d::ResMesh>& meshes)
{
    MeshLoader loader;
    return loader.Load(path, meshes);
}

} // namespace a3dx
