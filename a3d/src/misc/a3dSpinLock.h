//-------------------------------------------------------------------------------------------------
// File : a3dSpinLock.h
// Desc : Spin Lock.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//------------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------------
#include <xmmintrin.h>


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SpinLock class
///////////////////////////////////////////////////////////////////////////////////////////////////
class SpinLock
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
    //! @brief      ロックします.
    //---------------------------------------------------------------------------------------------
    void lock()
    {
        while(m_State.test_and_set(std::memory_order_acquire))
        { _mm_pause(); }
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ロックを解除します.
    //---------------------------------------------------------------------------------------------
    void unlock()
    { m_State.clear(std::memory_order_release); }

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    std::atomic_flag m_State = ATOMIC_FLAG_INIT;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// LockGuard class
///////////////////////////////////////////////////////////////////////////////////////////////////
class LockGuard
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
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    LockGuard(SpinLock* pLock)
    {
        m_pSpinLock = pLock;
        if (m_pSpinLock != nullptr)
        { m_pSpinLock->lock(); }
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~LockGuard()
    {
        if (m_pSpinLock != nullptr)
        { m_pSpinLock->unlock(); }
    }

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    SpinLock*   m_pSpinLock = nullptr;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace a3d