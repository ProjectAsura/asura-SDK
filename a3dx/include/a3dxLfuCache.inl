//-------------------------------------------------------------------------------------------------
// File : a3dxLfuCache.inl
// Desc : Least Frequency Used Cache Moudle.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace a3dx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// LfuCache class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
template<typename T, class Allocator> inline
LfuCache<T, Allocator>::LfuCache(size_t capacity)
: m_Capacity(capacity)
, m_Cache()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
template<typename T, class Allocator> inline
LfuCache<T, Allocator>::~LfuCache()
{ Clear(); }

//-------------------------------------------------------------------------------------------------
//      要素を追加します.
//-------------------------------------------------------------------------------------------------
template<typename T, class Allocator> inline
void LfuCache<T, Allocator>::Add(const T& item)
{
    if ( Contains(item) )
    {
        m_Cache[item]++;
    }
    else if ( m_Cache.size() < m_Capacity )
    {
        m_Cache[item] = 1;
    }
    else
    {
        auto iter = m_Cache.begin();
        auto mini = (*iter).second;

        for(auto it = m_Cache.begin(); it != m_Cache.end(); ++it )
        {
            if ((*it).second < mini )
            {
                mini = (*it).second;
                iter = it;
            }
        }

        m_Cache.erase(iter);
        m_Cache[item] = 1;
    }
}

//-------------------------------------------------------------------------------------------------
//      要素を削除します.
//-------------------------------------------------------------------------------------------------
template<typename T, class Allocator> inline
void LfuCache<T, Allocator>::Remove(const T& item)
{
    auto iter = m_Cache.find(item);
    if (iter != m_Cache.end())
    { m_Cache.erase(iter); }
}

//-------------------------------------------------------------------------------------------------
//      全要素を削除します.
//-------------------------------------------------------------------------------------------------
template<typename T, class Allocator> inline
void LfuCache<T, Allocator>::Clear()
{ m_Cache.clear(); }

//-------------------------------------------------------------------------------------------------
//      要素が含まれているか判定します.
//-------------------------------------------------------------------------------------------------
template<typename T, class Allocator> inline
bool LfuCache<T, Allocator>::Contains(const T& item) const
{ return m_Cache.find(item) != m_Cache.cend(); }

//-------------------------------------------------------------------------------------------------
//      配列にコピーします.
//-------------------------------------------------------------------------------------------------
template<typename T, class Allocator> inline
void LfuCache<T, Allocator>::Copy(T* pArray, size_t offset) const
{
    for( auto itr = m_Cache.cbegin(); itr != m_Cache.cend(); ++itr )
    {
        pArray[offset] = (*itr).first;
        offset++;
    }
}

//-------------------------------------------------------------------------------------------------
//      最大収容可能数を取得します.
//-------------------------------------------------------------------------------------------------
template<typename T, class Allocator> inline
size_t LfuCache<T, Allocator>::GetCapacity() const
{ return m_Capacity; }

//-------------------------------------------------------------------------------------------------
//      現在の収容数を取得します.
//-------------------------------------------------------------------------------------------------
template<typename T, class Allocator> inline
size_t LfuCache<T, Allocator>::GetCount() const
{ return m_Cache.size(); }

} // namespace a3dx
