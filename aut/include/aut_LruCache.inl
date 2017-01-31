﻿//-------------------------------------------------------------------------------------------------
// File : aut_LruCache.inl
// Desc : Least Recently Used Cache Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


namespace aut {

///////////////////////////////////////////////////////////////////////////////////////////////////
// LruCache class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
template<typename T> inline
LruCache<T>::LruCache(size_t capacity)
: m_Cache()
, m_Capacity(capacity)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
template<typename T> inline
LruCache<T>::~LruCache()
{ Clear(); }

//-------------------------------------------------------------------------------------------------
//      要素を追加します.
//-------------------------------------------------------------------------------------------------
template<typename T> inline
void LruCache<T>::Add(const T& item)
{
    if ( Contains(item) )
    {
        m_Cache.remove(item);
        m_Cache.push_back(item);
    }
    else if ( m_Cache.size() < m_Capacity )
    {
        m_Cache.push_back(item);
    }
    else
    {
        m_Cache.pop_front();
        m_Cache.push_back(item);
    }
}

//-------------------------------------------------------------------------------------------------
//      要素を削除します.
//-------------------------------------------------------------------------------------------------
template<typename T> inline
void LruCache<T>::Remove(const T& item)
{ m_Cache.remove(item); }

//-------------------------------------------------------------------------------------------------
//      全要素を削除します.
//-------------------------------------------------------------------------------------------------
template<typename T> inline
void LruCache<T>::Clear()
{ m_Cache.clear(); }

//-------------------------------------------------------------------------------------------------
//      要素が含まれているか判定します.
//-------------------------------------------------------------------------------------------------
template<typename T> inline
bool LruCache<T>::Contains(const T& item) const
{ return std::find(m_Cache.cbegin(), m_Cache.cend(), item) != m_Cache.cend(); }

//-------------------------------------------------------------------------------------------------
//      配列にコピーします.
//-------------------------------------------------------------------------------------------------
template<typename T> inline
void LruCache<T>::Copy(T* pArray, size_t offset) const
{
    for( auto itr = m_Cache.cbegin(); itr != m_Cache.cend(); itr++ )
    {
        pArray[offset] = (*itr);
        offset++;
    }
}

//-------------------------------------------------------------------------------------------------
//      最大収容可能数を取得します.
//-------------------------------------------------------------------------------------------------
template<typename T> inline
size_t LruCache<T>::GetCapacity() const
{ return m_Capacity; }

//-------------------------------------------------------------------------------------------------
//      現在の収容数を取得します.
//--------------------------------------------------------------------------------------------------
template<typename T> inline
size_t LruCache<T>::GetCount() const
{ return m_Cache.size(); }

} // namespace aut