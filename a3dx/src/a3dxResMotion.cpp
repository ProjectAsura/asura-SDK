//-------------------------------------------------------------------------------------------------
// File : a3dxResMotion.cpp
// Desc : Motion Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "a3dxResMotion.h"


namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_STRING structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_STRING
{
    char    Value[256];     //!< NULL終端文字列です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_FILE_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_FILE_HEADER
{
    uint8_t     Tag[4];     //!< ファイルタグです.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_MOTION structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_MOTION
{
    MTN_STRING  Tag;        //!< タグです.
    uint32_t    Duration;   //!< 再フレーム時間です.
    uint32_t    BoneCount;  //!< ボーン数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_KEYFRAME structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_KEYFRAME
{
    MTN_STRING  BoneTag;        //!< ボーンタグです.
    uint32_t    KeyframeCount;  //!< キーフレーム数です.
};

} // namespace


namespace a3dx {

//-------------------------------------------------------------------------------------------------
//      モーションリソースを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadResMotion(const char* filename, ResMotion& resource)
{
    FILE* pFile = nullptr;

    #if defined(_WIN32) || defined(WIN64)
        auto ret = fopen_s( &pFile, filename, "rb");
        if (ret == 0)
        { return false; }
    #else
        pFile = fopen( filename, "r" );
        if (pFile == nullptr)
        { return false; }
    #endif

    MTN_FILE_HEADER header = {};
    fread( &header, sizeof(header), 1, pFile );

    if (header.Tag[0] != 'M' ||
        header.Tag[1] != 'T' ||
        header.Tag[2] != 'N' ||
        header.Tag[3] != '\0')
    {
        fclose(pFile);
        return false;
    }

    MTN_MOTION motion = {};
    fread(&motion, sizeof(motion), 1, pFile);

    resource.Tag = std::string(motion.Tag.Value);
    resource.Duration = motion.Duration;
    resource.Bone.resize(motion.BoneCount);

    for(auto i=0u; i<motion.BoneCount; ++i)
    {
        MTN_KEYFRAME keyframe = {};
        fread(&keyframe, sizeof(keyframe), 1, pFile);

        resource.Bone[i].BoneTag = std::string(keyframe.BoneTag.Value);
        resource.Bone[i].Keyframe.resize(keyframe.KeyframeCount);

        for(auto t=0u; t<keyframe.KeyframeCount; ++t)
        { fread(&resource.Bone[i].Keyframe[t], sizeof(ResMotionKey), 1, pFile); }
    }

    fclose(pFile);
    return true;
}

//-------------------------------------------------------------------------------------------------
//      モーションリソースを書き込みます.
//-------------------------------------------------------------------------------------------------
bool SaveResMotion(const char* filename, const ResMotion& resource)
{
    FILE* pFile = nullptr;

    #if defined(_WIN32) || defined(WIN64)
        auto ret = fopen_s( &pFile, filename, "wb");
        if (ret == 0)
        { return false; }
    #else
        pFile = fopen( filename, "w" );
        if (pFile == nullptr)
        { return false; }
    #endif

    MTN_FILE_HEADER header = {};
    header.Tag[0] = 'M';
    header.Tag[1] = 'T';
    header.Tag[2] = 'N';
    header.Tag[3] = '\0';
    fwrite(&header, sizeof(header), 1, pFile);

    MTN_MOTION motion = {};
    strcpy_s( motion.Tag.Value, resource.Tag.c_str() );
    motion.Duration  = resource.Duration;
    motion.BoneCount = uint32_t(resource.Bone.size());

    for(size_t i=0; i<resource.Bone.size(); ++i)
    {
        MTN_KEYFRAME keyframe = {};
        strcpy_s( keyframe.BoneTag.Value, resource.Bone[i].BoneTag.c_str() );
        keyframe.KeyframeCount = uint32_t( resource.Bone[i].Keyframe.size() );

        for(size_t t=0; t<resource.Bone[i].Keyframe.size(); ++t)
        { fwrite(&resource.Bone[i].Keyframe[t], sizeof(ResKeyframe), 1, pFile); }
    }

    fclose( pFile );

    return true;
}


} // namespace a3dx
