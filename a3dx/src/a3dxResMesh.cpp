//-----------------------------------------------------------------------------
// File : a3dxResMesh.cpp
// Desc : Mesh Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <a3dxResMesh.h>
#include <cstdio>


namespace {

///////////////////////////////////////////////////////////////////////////////
// MeshHeader structure
///////////////////////////////////////////////////////////////////////////////
struct MeshHeader
{
    uint8_t     Magic[4];
    uint32_t    CountPosition;
    uint32_t    CountColor;
    uint32_t    CountTexCoord0;
    uint32_t    CountTexCoord1;
    uint32_t    CountTexCoord2;
    uint32_t    CountTexCoord3;
    uint32_t    CountNormal;
    uint32_t    CountTangent;
    uint32_t    CountBoneIndex;
    uint32_t    CountBoneWeight;
    uint32_t    CountVertexIndex;
    uint32_t    CountSubset;
    uint32_t    CountMaterial;
};

template<typename T>
bool AllocAndRead(a3d::IAllocator* pAllocator, FILE* pFile, uint32_t count, T* pDst)
{
    if (count == 0)
    { return true; }

    pDst = static_cast<T*>(pAllocator->Alloc(sizeof(T) * count, 4));
    if (pDst == nullptr)
    { return false; }

    fread(pDst, sizeof(T), count, pFile);
    return true;
}

} // namespace

namespace a3d {

///////////////////////////////////////////////////////////////////////////////
// ResMesh class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ResMesh::ResMesh()
: CountPosition     (0)
, CountColor        (0)
, CountTexCoord0    (0)
, CountTexCoord1    (0)
, CountTexCoord2    (0)
, CountTexCoord3    (0)
, CountNormal       (0)
, CountTangent      (0)
, CountBoneIndex    (0)
, CountBoneWeight   (0)
, CountVertexIndex  (0)
, CountSubset       (0)
, CountMaterial     (0)
, pPosition         (nullptr)
, pColor            (nullptr)
, pTexCoord0        (nullptr)
, pTexCoord1        (nullptr)
, pTexCoord2        (nullptr)
, pTexCoord3        (nullptr)
, pNormal           (nullptr)
, pTangent          (nullptr)
, pBoneIndex        (nullptr)
, pBoneWeight       (nullptr)
, pVertexIndex      (nullptr)
, pSubset           (nullptr)
, pMaterial         (nullptr)
, m_pAllocator      (nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      読み込み処理を行います.
//-----------------------------------------------------------------------------
bool ResMesh::Load(IAllocator* pAllocator, const char* path)
{
    FILE* pFile = nullptr;
    pFile = fopen(path, "rb");
    if (pFile == nullptr)
    { return false; }

    MeshHeader header;
    fread(&header, sizeof(header), 1, pFile);

    if (header.Magic[0] != 'M'
     || header.Magic[1] != 'S'
     || header.Magic[2] != 'H' 
     || header.Magic[3] != '\0')
    {
        fclose(pFile);
        return false;
    }

    CountPosition       = header.CountPosition;
    CountColor          = header.CountColor;
    CountTexCoord0      = header.CountTexCoord0;
    CountTexCoord1      = header.CountTexCoord1;
    CountTexCoord2      = header.CountTexCoord2;
    CountTexCoord3      = header.CountTexCoord3;
    CountNormal         = header.CountNormal;
    CountTangent        = header.CountTangent;
    CountBoneIndex      = header.CountBoneIndex;
    CountBoneWeight     = header.CountBoneWeight;
    CountVertexIndex    = header.CountVertexIndex;
    CountSubset         = header.CountSubset;
    CountMaterial       = header.CountMaterial;

    m_pAllocator = pAllocator;

    if (!AllocAndRead(pAllocator, pFile, CountPosition, pPosition))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountColor, pColor))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountTexCoord0, pTexCoord0))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountTexCoord1, pTexCoord1))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountTexCoord2, pTexCoord2))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountTexCoord3, pTexCoord3))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountNormal, pNormal))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountTangent, pTangent))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountBoneIndex, pBoneIndex))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountBoneWeight, pBoneWeight))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountVertexIndex, pVertexIndex))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountSubset, pSubset))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    if (!AllocAndRead(pAllocator, pFile, CountMaterial, pMaterial))
    {
        Dispose();
        fclose(pFile);
        return false;
    }

    fclose(pFile);
    return true;
}

//-----------------------------------------------------------------------------
//      書き込み処理を行います.
//-----------------------------------------------------------------------------
bool ResMesh::Save(const char* path)
{
    FILE* pFile = nullptr;
    pFile = fopen(path, "wb");
    if (pFile == nullptr)
    { return false; }

    MeshHeader header;
    header.Magic[0]         = 'M';
    header.Magic[1]         = 'S';
    header.Magic[2]         = 'H';
    header.Magic[3]         = '\0';    
    header.CountPosition    = CountPosition;
    header.CountColor       = CountColor;
    header.CountTexCoord0   = CountTexCoord0;
    header.CountTexCoord1   = CountTexCoord1;
    header.CountTexCoord2   = CountTexCoord2;
    header.CountTexCoord3   = CountTexCoord3;
    header.CountNormal      = CountNormal;
    header.CountTangent     = CountTangent;
    header.CountBoneIndex   = CountBoneIndex;
    header.CountBoneWeight  = CountBoneWeight;
    header.CountVertexIndex = CountVertexIndex;
    header.CountSubset      = CountSubset;
    header.CountMaterial    = CountMaterial;

    if (CountPosition > 0)
    { fwrite(pPosition, sizeof(Vector3), CountPosition, pFile); }

    if (CountColor > 0)
    { fwrite(pColor, sizeof(Vector4), CountColor, pFile); }

    if (CountTexCoord0 > 0)
    { fwrite(pTexCoord0, sizeof(Vector2), CountTexCoord0, pFile); }

    if (CountTexCoord1 > 0)
    { fwrite(pTexCoord1, sizeof(Vector2), CountTexCoord1, pFile); }

    if (CountTexCoord2 > 0)
    { fwrite(pTexCoord2, sizeof(Vector2), CountTexCoord2, pFile); }

    if (CountTexCoord3 > 0)
    { fwrite(pTexCoord3, sizeof(Vector2), CountTexCoord3, pFile); }

    if (CountNormal > 0)
    { fwrite(pNormal, sizeof(Vector3), CountNormal, pFile); }

    if (CountTangent > 0)
    { fwrite(pTangent, sizeof(Vector4), CountTangent, pFile); }

    if (CountBoneIndex > 0)
    { fwrite(pBoneIndex, sizeof(BoneIndex), CountBoneIndex, pFile); }

    if (CountBoneWeight > 0)
    { fwrite(pBoneWeight, sizeof(Vector4), CountBoneWeight, pFile); }

    if (CountVertexIndex > 0)
    { fwrite(pVertexIndex, sizeof(uint32_t), CountVertexIndex, pFile); }

    if (CountSubset > 0)
    { fwrite(pSubset, sizeof(ResSubset), CountSubset, pFile); }

    if (CountMaterial > 0)
    { fwrite(pMaterial, sizeof(MaterialTag), CountMaterial, pFile); }

    fclose(pFile);
    return true;
}

//-----------------------------------------------------------------------------
//      破棄処理を行います.
//-----------------------------------------------------------------------------
void ResMesh::Dispose()
{
    if (m_pAllocator != nullptr)
    {
        auto ptr = pPosition;
        m_pAllocator->Free(ptr);
        pPosition = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pColor;
        m_pAllocator->Free(ptr);
        pColor = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pTexCoord0;
        m_pAllocator->Free(ptr);
        pTexCoord0 = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pTexCoord1;
        m_pAllocator->Free(ptr);
        pTexCoord1 = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pTexCoord2;
        m_pAllocator->Free(ptr);
        pTexCoord2 = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pTexCoord3;
        m_pAllocator->Free(ptr);
        pTexCoord3 = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pNormal;
        m_pAllocator->Free(ptr);
        pNormal = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pTangent;
        m_pAllocator->Free(ptr);
        pTangent = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pBoneIndex;
        m_pAllocator->Free(ptr);
        pBoneIndex = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pBoneWeight;
        m_pAllocator->Free(ptr);
        pBoneWeight = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pVertexIndex;
        m_pAllocator->Free(ptr);
        pVertexIndex = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pSubset;
        m_pAllocator->Free(ptr);
        pSubset = nullptr;
    }

    if (m_pAllocator != nullptr)
    {
        auto ptr = pMaterial;
        m_pAllocator->Free(ptr);
        pMaterial = nullptr;
    }

    CountPosition       = 0;
    CountColor          = 0;
    CountTexCoord0      = 0;
    CountTexCoord1      = 0;
    CountTexCoord2      = 0;
    CountNormal         = 0;
    CountTangent        = 0;
    CountBoneIndex      = 0;
    CountBoneWeight     = 0;
    CountVertexIndex    = 0;
    CountSubset         = 0;
    CountMaterial       = 0;

    m_pAllocator = nullptr;
}

} // namespace a3d
