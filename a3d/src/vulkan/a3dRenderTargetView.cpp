﻿//-------------------------------------------------------------------------------------------------
// File : a3dRenderTargetView.cpp
// Desc : Texture View Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderTargetView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
RenderTargetView::RenderTargetView()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_pTexture        (nullptr)
, m_ImageView       (null_handle)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
RenderTargetView::~RenderTargetView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool RenderTargetView::Init(IDevice* pDevice, ITexture* pTexture, const TargetViewDesc* pDesc)
{
    if (pDevice == nullptr || pTexture == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pWrapTexture = static_cast<Texture*>(pTexture);
    A3D_ASSERT( pWrapTexture != nullptr );
    m_pTexture = pWrapTexture;
    m_pTexture->AddRef();

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT( pNativeDevice != null_handle );

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    // イメージビューを生成します.
    {
        VkImageViewCreateInfo info = {};
        info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext        = nullptr;
        info.flags        = 0;
        info.image        = m_pTexture->GetVkImage();
        info.viewType     = ToNativeImageViewType(pDesc->Dimension);
        info.format       = ToNativeFormat(pDesc->Format);
        info.components.r = VK_COMPONENT_SWIZZLE_R;
        info.components.g = VK_COMPONENT_SWIZZLE_G;
        info.components.b = VK_COMPONENT_SWIZZLE_B;
        info.components.a = VK_COMPONENT_SWIZZLE_A;
        info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel   = pDesc->MipSlice;
        info.subresourceRange.levelCount     = pDesc->MipLevels;
        info.subresourceRange.baseArrayLayer = pDesc->FirstArraySlice;
        info.subresourceRange.layerCount     = pDesc->ArraySize;

        auto ret = vkCreateImageView(pNativeDevice, &info, nullptr, &m_ImageView);
        if ( ret != VK_SUCCESS )
        {
            A3D_LOG("Error : vkCreateImageView() Failed. VkResult = %s", ToString(ret));
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT( pNativeDevice != null_handle );

    if ( m_ImageView != null_handle )
    {
        vkDestroyImageView( pNativeDevice, m_ImageView, nullptr );
        m_ImageView = null_handle;
    }

    a3d::SafeRelease( m_pTexture );
    a3d::SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t RenderTargetView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::SetName(const char* name)
{
    m_Name = name;
    if (vkDebugMarkerSetObjectName != nullptr)
    {
        auto pWrapDevice = static_cast<Device*>(m_pDevice);
        A3D_ASSERT(pWrapDevice != nullptr);

        VkDebugMarkerObjectNameInfoEXT info = {};
        info.sType          = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        info.objectType     = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT;
        info.object         = uint64_t(m_ImageView);
        info.pObjectName    = name;

        vkDebugMarkerSetObjectName(pWrapDevice->GetVkDevice(), &info);
    }
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* RenderTargetView::GetName() const
{ return m_Name.c_str(); }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      テクスチャの構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TextureDesc RenderTargetView::GetTextureDesc() const
{
    if ( m_pTexture == nullptr )
    {
        TextureDesc desc = {};
        return desc;
    }

    return m_pTexture->GetDesc();
}

//-------------------------------------------------------------------------------------------------
//      イメージを取得します.
//-------------------------------------------------------------------------------------------------
VkImage RenderTargetView::GetVkImage() const
{
    if ( m_pTexture == nullptr )
    { return null_handle; }

    return m_pTexture->GetVkImage(); 
}

//-------------------------------------------------------------------------------------------------
//      イメージビューを取得します.
//-------------------------------------------------------------------------------------------------
VkImageView RenderTargetView::GetVkImageView() const
{ return m_ImageView; }

//-------------------------------------------------------------------------------------------------
//      テクスチャビューの構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetViewDesc IRenderTargetView::GetDesc() const
{
    auto pThis = static_cast<const RenderTargetView*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_Desc;
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ITexture* IRenderTargetView::GetResource() const
{
    auto pThis = static_cast<const RenderTargetView*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_pTexture;
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool RenderTargetView::Create
(
    IDevice*                pDevice,
    ITexture*               pTexture,
    const TargetViewDesc*   pDesc,
    IRenderTargetView**     ppView
)
{
    if (pDevice == nullptr || pTexture == nullptr || pDesc == nullptr || ppView == nullptr)
    {
        return false; }

    auto instance = new RenderTargetView();
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pTexture, pDesc) )
    {
        a3d::SafeRelease(instance);
        return false;
    }

    *ppView = instance;
    return true;
}

} // namespace a3d

