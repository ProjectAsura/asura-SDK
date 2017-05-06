//-------------------------------------------------------------------------------------------------
// File : rsResMotion.cpp
// Desc : Motion Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "rsResMotion.h"


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
    uint32_t    Count;      //!< モーション数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_MOTION structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_MOTION
{
    MTN_STRING  Tag;        //!< タグです.
    double      Duration;   //!< 再フレーム時間です.
    uint32_t    NodeCount;  //!< ボーン数です.
    uint32_t    MorphCount; //!< モーフ数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_NODE structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_NODE
{
    MTN_STRING  BoneTag;        //!< ボーンタグです.
    uint32_t    KeyframeCount;  //!< キーフレーム数です.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MTN_MORPH structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct MTN_MORPH
{
    MTN_STRING  Tag;            //!< タグです.
    uint32_t    KeyframeCount;  //!< キーフレーム数です.
};

} // namespace


namespace rs {

//-------------------------------------------------------------------------------------------------
//      モーションリソースを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadResMotion(const char* filename, std::vector<ResMotion>& resource)
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

    resource.resize(header.Count);

    for(auto c=0u; c<header.Count; ++c)
    {
        MTN_MOTION motion = {};
        fread(&motion, sizeof(motion), 1, pFile);

        resource[c].Tag        = std::string(motion.Tag.Value);
        resource[c].Duration   = motion.Duration;
        resource[c].Node.resize(motion.NodeCount);
        resource[c].Morph.resize(motion.MorphCount);

        for(auto i=0u; i<motion.NodeCount; ++i)
        {
            MTN_NODE keyframe = {};
            fread(&keyframe, sizeof(keyframe), 1, pFile);

            resource[c].Node[i].BoneTag = std::string(keyframe.BoneTag.Value);
            resource[c].Node[i].Keyframe.resize(keyframe.KeyframeCount);

            for(auto t=0u; t<keyframe.KeyframeCount; ++t)
            { fread(&resource[c].Node[i].Keyframe[t], sizeof(ResNodeKey), 1, pFile); }
        }

        for(auto i=0u; i<motion.MorphCount; ++i)
        {
            MTN_MORPH info = {};
            fread(&info, sizeof(info), 1, pFile);

            resource[c].Morph[i].Tag = std::string(info.Tag.Value);
            resource[c].Morph[i].Keyframe.resize(info.KeyframeCount);

            for(auto j=0u; j<info.KeyframeCount; ++j)
            { fread(&resource[c].Morph[i].Keyframe[j], sizeof(ResMorphKey), 1, pFile); }
        }
    }

    fclose(pFile);
    return true;
}

//-------------------------------------------------------------------------------------------------
//      モーションリソースを書き込みます.
//-------------------------------------------------------------------------------------------------
bool SaveResMotion(const char* filename, const std::vector<ResMotion>& resource)
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
    header.Count = uint32_t(resource.size());
    fwrite(&header, sizeof(header), 1, pFile);

    for(size_t c=0; c<resource.size(); ++c)
    {
        MTN_MOTION motion = {};
        strcpy_s( motion.Tag.Value, resource[c].Tag.c_str() );
        motion.Duration   = resource[c].Duration;
        motion.NodeCount  = uint32_t(resource[c].Node.size());
        motion.MorphCount = uint32_t(resource[c].Morph.size());

        for(size_t i=0; i<resource[c].Node.size(); ++i)
        {
            MTN_NODE node = {};
            strcpy_s( node.BoneTag.Value, resource[c].Node[i].BoneTag.c_str() );
            node.KeyframeCount = uint32_t( resource[c].Node[i].Keyframe.size() );

            for(size_t t=0; t<resource[c].Node[i].Keyframe.size(); ++t)
            { fwrite(&resource[c].Node[i].Keyframe[t], sizeof(ResNodeKey), 1, pFile); }
        }

        for(size_t i=0; i<resource[c].Morph.size(); ++i)
        {
            MTN_MORPH info = {};
            strcpy_s( info.Tag.Value, resource[c].Morph[i].Tag.c_str() );
            info.KeyframeCount = uint32_t(resource[c].Morph[i].Keyframe.size());
            fwrite(&info, sizeof(info), 1, pFile);

            for(size_t j=0; j<resource[c].Morph[i].Keyframe.size(); ++j)
            { fwrite(&resource[c].Morph[i].Keyframe[j], sizeof(ResMorphKey), 1, pFile); }
        }
    }

    fclose( pFile );

    return true;
}

} // namespace rs
