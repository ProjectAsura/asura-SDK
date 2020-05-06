﻿//-----------------------------------------------------------------------------
// File : a3dNsObject.mm
// Desc : NSObject Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <a3dNsObject.h>
#include <CoreFoundation/CFBase.h>


namespace a3d {

///////////////////////////////////////////////////////////////////////////////
// NsObject class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
NsObject::NSObject()
: m_Ptr(nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
NsObject::NsObject(const NsHandle& handle)
: m_Ptr(handle.ptr)
{
    if (m_Ptr)
    { CFRetain(m_Ptr); }
}

//-----------------------------------------------------------------------------
//      コピーコンストラクタです.
//-----------------------------------------------------------------------------
NsObject::NsObject(const NsObject& value)
: m_Ptr(value.m_Ptr)
{
    if (m_Ptr)
    { CFRetain(m_Ptr); }
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
NsObject::~NsObject()
{
    if (m_Ptr)
    { CFRelease(m_Ptr); }
}

//-----------------------------------------------------------------------------
//      代入演算子です.
//-----------------------------------------------------------------------------
NsObject& NsObject::operator = (const NsObject& value)
{
    if (value.m_Ptr == m_Ptr)
    { return *this; }

    if (value.m_Ptr)
    { CFRetain(value.m_Ptr); }

    if (m_Ptr)
    { CFRelease(m_Ptr); }

    m_Ptr = value.m_Ptr;
    return *this;
}

//-----------------------------------------------------------------------------
//      有効かどうかチェックします.
//-----------------------------------------------------------------------------
bool NsObject::IsValid() const
{ return m_Ptr != nullptr; }

} // namespace a3d