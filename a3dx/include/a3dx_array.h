//-------------------------------------------------------------------------------------------------
// File : a3dx_array.h
// Desc : Array Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3d.h>
#include <new>


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Array class
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class Array
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

    //---------------------------------------------------------------------------------------------
    //! @brief      �R���X�g���N�^�ł�.
    //---------------------------------------------------------------------------------------------
    Array()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      �f�X�g���N�^�ł�.
    //---------------------------------------------------------------------------------------------
    ~Array()
    { term(); }

    //---------------------------------------------------------------------------------------------
    //! @brief      �������������s���܂�.
    //!
    //! @param[in]      pAllocator      �A���P�[�^�ł�.
    //! @param[in]      count           �m�ۂ���A�C�e�����ł�.
    //! @retval true    �������ɐ���.
    //! @retval false   �������Ɏ��s.
    //---------------------------------------------------------------------------------------------
    bool init(IAllocator* pAlloator, uint32_t count)
    {
        if (pAlloator == nullptr || count <= 0)
        { return false; }

        auto buf = m_pAllocator->Alloc(sizeof(T) * count, alignof(T));
        if (buf == nullptr)
        { return false; }

        m_pAllocator = pAlloator;
        m_Count      = count;
        m_pItems     = new(buf) T[count];

        return true;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      �I���������s���܂�.
    //---------------------------------------------------------------------------------------------
    void term()
    {
        if (m_pItems != nullptr && m_pAllocator != nullptr)
        {
            auto buf        = m_pItems;
            auto pAllocator = m_pAllocator;
            m_pItems->~T();
            pAllocator->Free(buf);
        }

        m_Count         = 0;
        m_pItems        = nullptr;
        m_pAllocator    = nullptr;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      �T�C�Y���擾���܂�.
    //!
    //! @return     �T�C�Y��ԋp���܂�.
    //---------------------------------------------------------------------------------------------
    uint32_t size() const
    { return m_Count; }

    //---------------------------------------------------------------------------------------------
    //! @brief      �擪�f�[�^���擾���܂�.
    //!
    //! @return     �擪�f�[�^��ԋp���܂�.
    //---------------------------------------------------------------------------------------------
    T* data() const
    { return m_pItems; }

    //---------------------------------------------------------------------------------------------
    //! @brief      �C���f�N�T�ł�.
    //!
    //! @param[in]      index       �z��ԍ��ł�.
    //! @return     �w�肳�ꂽ�z��ԍ��ɑΉ�����A�C�e����ԋp���܂�.
    //---------------------------------------------------------------------------------------------
    T& operator[] (int index)
    { return m_pItems[index]; }

    //---------------------------------------------------------------------------------------------
    //! @brief      �C���f�N�T�ł�.
    //!
    //! @param[in]      index       �z��ԍ��ł�.
    //! @return     �w�肳�ꂽ�z��ԍ��ɑΉ�����A�C�e����ԋp���܂�.
    //---------------------------------------------------------------------------------------------
    const T& operator[] (int index) const
    { return m_pItems[index]; }

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    IAllocator*     m_pAllocator    = nullptr;      //!< �A���P�[�^�ł�.
    uint32_t        m_Count         = 0;            //!< �A�C�e�����ł�.
    T*              m_pItems        = nullptr;      //!< �A�C�e���ł�.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace a3d