//-------------------------------------------------------------------------------------------------
// File : a3dx_resmesh.cpp
// Desc : Mesh Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3dx_resmesh.h>
#include <cstdio>


namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MshHeader structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MshHeader
{
    uint32_t    Magic;  // ' HSM';
    uint32_t    Count;  // メッシュ数.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MeshDesc structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MeshDesc
{
    uint32_t PositionCount;
    uint32_t NormalCount;
    uint32_t TangentCount;
    uint32_t TexCoordCount;
    uint32_t IndexCount;
    char     MaterialTag[64];
};

} // namespace


namespace a3d {

//-------------------------------------------------------------------------------------------------
//          MSHファイルを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadMesh(IAllocator* pAllocator, const char* path, Array<ResMesh>& meshes)
{
    FILE* pFile = fopen(path, "rb");
    if (pFile == nullptr)
    { return false; }

    MshHeader header = {};
    fread(&header, sizeof(header), 1, pFile);

    if (header.Magic != ' HSM')
    {
        fclose(pFile);
        return false;
    }

    if (!meshes.init(pAllocator, header.Count))
    {
        fclose(pFile);
        return false;
    }

    for(auto i=0u; i<header.Count; ++i)
    {
        MeshDesc mesh;
        fread(&mesh, sizeof(mesh), 1, pFile);

        if (!meshes[i].Positions.init(pAllocator, mesh.PositionCount))
        {
            DisposeMesh(meshes);
            fclose(pFile);
            return false;
        }

        if (!meshes[i].Normals.init(pAllocator, mesh.NormalCount))
        {
            DisposeMesh(meshes);
            fclose(pFile);
            return false;
        }

        if (!meshes[i].Tangents.init(pAllocator, mesh.TangentCount))
        {
            DisposeMesh(meshes);
            fclose(pFile);
            return false;
        }

        if (!meshes[i].TexCoords.init(pAllocator, mesh.TexCoordCount))
        {
            DisposeMesh(meshes);
            fclose(pFile);
            return false;
        }

        if (!meshes[i].Indices.init(pAllocator, mesh.IndexCount))
        {
            DisposeMesh(meshes);
            fclose(pFile);
            return false;
        }

        meshes[i].MaterialTag = mesh.MaterialTag;
    }

    fclose(pFile);
    return true;
}

//-------------------------------------------------------------------------------------------------
//          MSHファイルに書き込みます.
//-------------------------------------------------------------------------------------------------
bool SaveMesh(const char* path, const Array<ResMesh>& meshes)
{
    FILE* pFile = fopen(path, "wb");
    if (pFile == nullptr)
    { return false; }

    MshHeader header = {};
    header.Magic = ' HSM';
    header.Count = meshes.size();

    for(auto i=0u; i<meshes.size(); ++i)
    {
        MeshDesc desc = {};
        desc.PositionCount  = meshes[i].Positions.size();
        desc.NormalCount    = meshes[i].Normals.size();
        desc.TangentCount   = meshes[i].Tangents.size();
        desc.TexCoordCount  = meshes[i].TexCoords.size();
        desc.IndexCount     = meshes[i].Indices.size();
        for(auto j=0; j<64; ++j)
        { desc.MaterialTag[j] = meshes[i].MaterialTag[j]; }

        fwrite(&desc, sizeof(desc), 1, pFile);
        fwrite(meshes[i].Positions.data(),  sizeof(Vector3)  * desc.PositionCount,  1, pFile);
        fwrite(meshes[i].Normals.data(),    sizeof(Vector3)  * desc.NormalCount,    1, pFile);
        fwrite(meshes[i].Tangents.data(),   sizeof(Vector3)  * desc.TangentCount,   1, pFile);
        fwrite(meshes[i].TexCoords.data(),  sizeof(Vector2)  * desc.TexCoordCount,  1, pFile);
        fwrite(meshes[i].Indices.data(),    sizeof(uint32_t) * desc.IndexCount,     1, pFile);
    }

    fclose(pFile);
    return true;
}

//-------------------------------------------------------------------------------------------------
//          メッシュを破棄します.
//-------------------------------------------------------------------------------------------------
void DisposeMesh(ResMesh& mesh)
{
    mesh.Positions  .term();
    mesh.Normals    .term();
    mesh.Tangents   .term();
    mesh.TexCoords  .term();
    mesh.Indices    .term();
}

//-------------------------------------------------------------------------------------------------
//          メッシュを破棄します.
//-------------------------------------------------------------------------------------------------
void DisposeMesh(Array<ResMesh>& mesh)
{
    for(auto i=0u; i<mesh.size(); ++i)
    { DisposeMesh(mesh[i]); }

    mesh.term();
}

} // namespace a3d
