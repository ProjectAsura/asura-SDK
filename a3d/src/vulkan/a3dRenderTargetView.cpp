//-------------------------------------------------------------------------------------------------
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
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    auto pWrapTexture = static_cast<Texture*>(pTexture);
    A3D_ASSERT( pWrapTexture != nullptr );
    m_pTexture = pWrapTexture;
    m_pTexture->AddRef();

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT( pNativeDevice != null_handle );

    memcpy( &m_Desc, pDesc, sizeof(m_Desc) );

    // イメージビューを生成します.
    {
        VkImageViewCreateInfo info = {};
        info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext        = nullptr;
        info.flags        = 0;
        info.image        = m_pTexture->GetVulkanImage();
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
        { return false; }
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

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
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
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void RenderTargetView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      テクスチャビューの構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetViewDesc RenderTargetView::GetDesc() const
{ return m_Desc; }

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
VkImage RenderTargetView::GetVulkanImage() const
{
    if ( m_pTexture == nullptr )
    { return null_handle; }

    return m_pTexture->GetVulkanImage(); 
}

//-------------------------------------------------------------------------------------------------
//      イメージビューを取得します.
//-------------------------------------------------------------------------------------------------
VkImageView RenderTargetView::GetVulkanImageView() const
{ return m_ImageView; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ITexture* RenderTargetView::GetResource() const
{ return m_pTexture; }

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
    { return false; }

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

