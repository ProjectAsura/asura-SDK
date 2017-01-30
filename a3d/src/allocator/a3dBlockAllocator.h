//-------------------------------------------------------------------------------------------------
// File : a3dBlockAllocator.h
// Desc : Block Allocator.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "container/a3dList.h"
#include <mutex>


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Block structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Block
{
    ptrdiff_t   Offset;     //!< オフセットです.
    size_t      Size;       //!< 確保サイズです.
    size_t      Alignment;  //!< アライメントです.
};

//-------------------------------------------------------------------------------------------------
//         等価比較演算子のオーバーロードです.
//-------------------------------------------------------------------------------------------------
inline bool operator == (const Block& lhs, const Block& rhs)
{ return lhs.Offset == rhs.Offset && lhs.Size == rhs.Size && lhs.Alignment == rhs.Alignment; }

//-------------------------------------------------------------------------------------------------
//        大小比較演算子のオーバーロードです.
//-------------------------------------------------------------------------------------------------
inline bool operator <  (const Block& lhs, const Block& rhs)
{ return lhs.Offset < rhs.Offset; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// BlockAllocator class
// ※オフセットとサイズのみ管理します. 実メモリは管理しません.
///////////////////////////////////////////////////////////////////////////////////////////////////
class BlockAllocator
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //---------------------------------------------------------------------------------------------
    // Using Alias
    //---------------------------------------------------------------------------------------------
    using Locker = std::lock_guard<std::mutex>;

    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    BlockAllocator()
    : m_Size     (0)
    , m_Offset   (0)
    , m_CurOffset(0)
    , m_UsedSize (0)
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~BlockAllocator()
    { Term(); }

    //---------------------------------------------------------------------------------------------
    //! @brief      初期化を行います.
    //---------------------------------------------------------------------------------------------
    bool Init(size_t size, ptrdiff_t offset)
    {
        m_Size      = size;
        m_Offset    = offset;
        m_CurOffset = 0;
        m_UsedSize  = 0;
        m_FreeBlocks.clear();
        return true;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //---------------------------------------------------------------------------------------------
    void Term()
    {
        Locker locker(m_Mutex);
        m_Size      = 0;
        m_Offset    = 0;
        m_CurOffset = 0;
        m_UsedSize  = 0;
        m_FreeBlocks.clear();
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ブロック確保可能かチェックします.
    //---------------------------------------------------------------------------------------------
    bool CanAlloc( size_t size, size_t alignment ) const
    {
        auto allocSize = RoundUp( size, alignment );
        if (m_CurOffset + allocSize <= m_Size)
        { return true; }

        for(auto& itr : m_FreeBlocks)
        {
            if (allocSize <= itr.Size)
            { return true; }
        }

        return false;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ブロックを確保します.
    //---------------------------------------------------------------------------------------------
    Block Alloc( size_t size, size_t alignment )
    {
        Locker locker(m_Mutex);

        auto allocSize = RoundUp( size, alignment );

        if (m_CurOffset + allocSize <= m_Size)
        {
            auto prevOffset = m_Offset + m_CurOffset;
            auto currOffset = RoundUp( prevOffset, alignment );

            Block result  = {};
            result.Size      = size;
            result.Offset    = currOffset;
            result.Alignment = alignment;

            auto usedSize = (currOffset - prevOffset) + size;
            m_CurOffset += usedSize;
            m_UsedSize  += usedSize;
            return result;
        }

        Block result = {};

        // 空いているブロックを探す.
        if (FindFreeBlock(size, alignment, result))
        { return result; }

        // 空きサイズが十分かどうかチェック.
        if (m_UsedSize + allocSize <= m_Size)
        {
            // 一回メモリコンパクションを行う.
            InnerCompact();

            // 綺麗になった状態で，もう一度確保できるか実行する.
            if (FindFreeBlock(size, alignment, result))
            { return result; }
        }

        // ダメだったら空ブロックを返す.
        Block empty = {};
        return empty;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ブロックを解放します.
    //---------------------------------------------------------------------------------------------
    void Free( const Block& block )
    {
        Locker locker(m_Mutex);

        auto offset   = RoundDown(block.Offset, block.Alignment);
        auto usedSize = (block.Offset - offset) + block.Size;

        Block freeBlock = {};
        freeBlock.Offset    = offset;
        freeBlock.Size      = usedSize;
        freeBlock.Alignment = 0;

        m_UsedSize -= usedSize;
        m_FreeBlocks.push_back(freeBlock);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリコンパクションを行います.
    //---------------------------------------------------------------------------------------------
    void Compact()
    {
        Locker locker(m_Mutex);
        InnerCompact();
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      全体のサイズを取得します.
    //---------------------------------------------------------------------------------------------
    size_t GetSize() const
    { return m_Size; }

    //---------------------------------------------------------------------------------------------
    //! @brief      先頭オフセットを取得します.
    //---------------------------------------------------------------------------------------------
    ptrdiff_t GetOffset() const
    { return m_Offset; }

    //---------------------------------------------------------------------------------------------
    //! @brief      使用サイズがゼロかどうかチェックします.
    //---------------------------------------------------------------------------------------------
    bool IsEmpty() const
    { return m_UsedSize == 0; }

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    size_t              m_Size;         //!< ブロック全体のサイズ.
    ptrdiff_t           m_Offset;       //!< 先頭オフセット.
    ptrdiff_t           m_CurOffset;    //!< 現在のオフセット.
    a3d::list<Block>    m_FreeBlocks;   //!< 解放済みブロック.
    std::mutex          m_Mutex;        //!< ミューテックス.
    size_t              m_UsedSize;     //!< 使用中のメモリサイズ.

    //=============================================================================================
    // private methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      指定された倍数に切り上げます.
    //---------------------------------------------------------------------------------------------
    static size_t RoundUp(size_t value, size_t base)
    { return ( value + ( base - 1 ) ) & ~( base - 1 ); }

    //---------------------------------------------------------------------------------------------
    //! @brief      指定された倍数に切り下げます.
    //---------------------------------------------------------------------------------------------
    static size_t RoundDown(size_t value, size_t base)
    { return value & ~( base - 1 ); }

    //---------------------------------------------------------------------------------------------
    //! @brief      メモリコンパクションを行います.
    //---------------------------------------------------------------------------------------------
    void InnerCompact()
    {
        m_FreeBlocks.sort();

        Block prev = {};
        auto itr = std::begin(m_FreeBlocks);

        while (itr != std::end(m_FreeBlocks))
        {
            if (prev.Offset + prev.Size == itr->Offset)
            {
                // ブロックを連結.
                Block comb = {};
                comb.Offset = prev.Offset;
                comb.Size   = prev.Size + itr->Size;

                itr = m_FreeBlocks.erase ( itr );
                itr = m_FreeBlocks.insert( itr, comb );
                m_FreeBlocks.remove( prev );

                prev.Offset = comb.Offset;
                prev.Size   = comb.Size;
            }
            else
            {
                prev.Offset = itr->Offset;
                prev.Size   = itr->Size;
                itr++;
            }
        }
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      空きブロックリストから割り当て可能なブロックを探します.
    //---------------------------------------------------------------------------------------------
    bool FindFreeBlock(size_t size, size_t alignment, Block& result)
    {
        auto allocSize = RoundUp( size, alignment );

        for(auto& itr : m_FreeBlocks)
        {
            if (allocSize <= itr.Size)
            {
                auto prevOffset = itr.Offset;
                auto currOffset = RoundUp( prevOffset, alignment );

                result.Size      = size;
                result.Offset    = currOffset;
                result.Alignment = alignment;

                auto usedSize = (currOffset - prevOffset) + size;

                m_UsedSize += usedSize;

                Block free  = {};
                free.Size       = itr.Size   - usedSize;
                free.Offset     = itr.Offset + usedSize;
                free.Alignment  = 0;

                m_FreeBlocks.remove( itr );
                m_FreeBlocks.push_back( free );
                return true;
            }
        }

        return false;
    }
};

} // namespace a3d
