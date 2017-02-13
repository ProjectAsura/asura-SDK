//-------------------------------------------------------------------------------------------------
// File : a3dxHeap.cpp
// Desc : Memory Heap.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3dxHeap.h>
#include <cassert>


namespace {

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
constexpr uint64_t SIGN_ALLOC = 0xA3DA110C; // "a3d alloc".
constexpr uint64_t SIGN_DEAD  = 0xDEADC0DE; // "dead code".

}

namespace a3dx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Heap class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Heap::Heap()
: m_Size        (0)
, m_BlockCount  (0)
, m_UsedSize    (0)
, m_Offset      (0)
, m_pBuffer     (nullptr)
, m_pHead       (nullptr)
, m_pTail       (nullptr)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Heap::~Heap()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Heap::Init(uint64_t size)
{
    Term();

    std::lock_guard<std::mutex> locker(m_Mutex);

    m_pBuffer = static_cast<uint8_t*>(malloc(size));
    if (m_pBuffer == nullptr)
    { return false; }

    m_Size          = size;
    m_BlockCount    = 0;
    m_UsedSize      = 0;
    m_Offset        = 0;
    m_pHead         = nullptr;
    m_pTail         = nullptr;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Heap::Term()
{
    if (m_pBuffer == nullptr)
    { return; }

    std::lock_guard<std::mutex> locker(m_Mutex);
    free(m_pBuffer);

    m_pBuffer       = nullptr;
    m_Size          = 0;
    m_BlockCount    = 0;
    m_UsedSize      = 0;
    m_Offset        = 0;
    m_pHead         = nullptr;
    m_pTail         = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      メモリを確保します.
//-------------------------------------------------------------------------------------------------
void* Heap::Alloc(size_t size, size_t alignment) noexcept
{
    // ゼロの場合はAlign()処理のために1を設定.
    if (alignment == 0)
    { alignment = 1; }

    // メモリブロック確保.
    auto ptr = AllocBlock(size + alignment);
    if (ptr == nullptr)
    { return nullptr; }

    // メモリアライメント処理をしたポインタを返却.
    return MemoryAlign(ptr, alignment);
}

//-------------------------------------------------------------------------------------------------
//      メモリを再確保します.
//-------------------------------------------------------------------------------------------------
void* Heap::Realloc(void* ptr, size_t size, size_t alignment) noexcept
{
    auto newPtr = Alloc(size, alignment);
    if (newPtr == nullptr)
    { return nullptr; }

    // ブロックヘッダ取得.
    auto temp   = reinterpret_cast<uint8_t*>(MemoryUnalign(ptr));
    auto pBlock = reinterpret_cast<Block*>(temp - sizeof(BlockHeader));

    // 最小サイズを求める.
    auto oldSize  = pBlock->Header.Size;
    auto copySize = (size < oldSize) ? size : oldSize;

    // メモリコピー.
    m_Mutex.lock();
    memcpy(newPtr, ptr, copySize);
    m_Mutex.unlock();

    // 要らなくなったので解放.
    Free(ptr);

    // 新しい領域を返却.
    return newPtr;
}

//-------------------------------------------------------------------------------------------------
//      メモリを解放します.
//-------------------------------------------------------------------------------------------------
void Heap::Free(void* ptr) noexcept
{
    if (ptr == nullptr)
    { return; }

    // ポインタを逆アライメント処理.
    ptr = MemoryUnalign(ptr);

    // メモリブロックを解放.
    FreeBlock(ptr);
}

//-------------------------------------------------------------------------------------------------
//      メモリマーカーを取得します.
//-------------------------------------------------------------------------------------------------
Heap::Info Heap::GetMarker()
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    Info result = {};
    result.Count = m_BlockCount;
    result.Size  = m_UsedSize;

    return result;
}

//-------------------------------------------------------------------------------------------------
//      メモリリークをチェックします.
//-------------------------------------------------------------------------------------------------
bool Heap::IsLeak(const Info& marker)
{ return IsLeak(marker, nullptr); }

//-------------------------------------------------------------------------------------------------
//      メモリリークをチェックします.
//-------------------------------------------------------------------------------------------------
bool Heap::IsLeak(const Info& marker, Info* pLeakInfo)
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    // 差分を求める.
    auto diffCount = m_BlockCount - marker.Count;
    auto diffSize  = m_UsedSize   - marker.Size;

    // ポインタが設定されていれば値を設定しておく.
    if (pLeakInfo != nullptr)
    {
        (*pLeakInfo).Count = diffCount;
        (*pLeakInfo).Size  = diffSize;
    }

    // 両方ともゼロであればリークしていないので，どちらかがゼロでなければメモリリークとなる.
    return (diffCount != 0 || diffSize != 0);
}

//-------------------------------------------------------------------------------------------------
//      メモリがいっぱいであるかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool Heap::IsFull()
{
    std::lock_guard<std::mutex> locker(m_Mutex);
    return m_UsedSize >= m_Size;
}

//-------------------------------------------------------------------------------------------------
//      メモリブロックを確保します.
//-------------------------------------------------------------------------------------------------
void* Heap::AllocBlock(uint64_t size)
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    if (size == 0)
    { return nullptr; }

    // サイズチェック.
    if (m_UsedSize + size > m_Size)
    { return nullptr; }

    void* result = nullptr;
    auto  offset = sizeof(BlockHeader);

    auto pPrev  = m_pTail;
    auto pBlock = reinterpret_cast<Block*>(m_pBuffer + m_Offset);

    // ヘッダに値を設定.
    pBlock->Header.Signature = SIGN_ALLOC;  // 生きている領域であることをマーク.
    pBlock->Header.Size      = size;
    pBlock->Header.pNext     = nullptr;
    pBlock->Header.pPrev     = pPrev;

    // メモリを設定.
    pBlock->pBuffer = static_cast<uint8_t*>(m_pBuffer + m_Offset + offset);

    // リンクリストの設定.
    if (pPrev != nullptr)
    { pPrev->Header.pNext = pBlock; }

    // 最初の処理の場合.
    if (m_pHead == nullptr)
    { m_pHead = pBlock; }

    // 末尾のポインタを設定.
    m_pTail = pBlock;

    m_UsedSize   += size;
    m_Offset     += offset + size;
    m_BlockCount++;

    // 返却メモリを設定.
    result = pBlock->pBuffer;

    return result;
}

//-------------------------------------------------------------------------------------------------
//      メモリブロックを解放します.
//-------------------------------------------------------------------------------------------------
void Heap::FreeBlock(void* ptr)
{
    std::lock_guard<std::mutex> locker(m_Mutex);
    auto offset = sizeof(BlockHeader);

    // オフセット分戻して，メモリブロックを復元.
    auto pBlock = reinterpret_cast<Block*>(reinterpret_cast<uint8_t*>(ptr) - offset);

    auto pNext = pBlock->Header.pNext;
    auto pPrev = pBlock->Header.pPrev;

    // リンクリストを繋ぎ直す.
    if (pPrev != nullptr)
    { pPrev->Header.pNext = pNext; }

    if (pNext != nullptr)
    { pNext->Header.pPrev = pPrev; }
    else
    {
        // 末尾の場合.
        if (pPrev != nullptr)
        {
            m_Offset = (reinterpret_cast<uint8_t*>(pPrev) - m_pBuffer) 
                       + pPrev->Header.Size + sizeof(BlockHeader);
            m_pTail  = pPrev;
        }
        // 先頭の場合.
        else
        {
            m_Offset = 0;
            m_pHead  = nullptr;
            m_pTail  = nullptr;
        }
    }

    // 解放処理なのでメモリサイズを減らす.
    m_UsedSize -= pBlock->Header.Size;

    // 解放したのでブロック数を減らす.
    m_BlockCount--;

    // リンクリストから外す.
    pBlock->Header.pNext = nullptr;
    pBlock->Header.pPrev = nullptr;

    // メモリ領域をクリアしておく.
    #if defined(DEBUG) || defined(_DEBUG)
        memset(pBlock->pBuffer, 0xf7, pBlock->Header.Size);
    #else
        memset(pBlock->pBuffer, 0x0, pBlock->Header.Size);
    #endif

    // ヘッダに値を設定.
    pBlock->Header.Signature = SIGN_DEAD;   // 死んでいる領域であることをマーク.
    pBlock->Header.Size      = 0;
    pBlock->pBuffer          = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      ポインタをアライメントします.
//-------------------------------------------------------------------------------------------------
void* Heap::MemoryAlign(void* ptr, size_t alignment)
{

    // TODO:

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
//      ポインタを逆アライメントします.
//-------------------------------------------------------------------------------------------------
void* Heap::MemoryUnalign(void* ptr)
{

    // TODO:

    return nullptr;
}

} // namespace a3dx
