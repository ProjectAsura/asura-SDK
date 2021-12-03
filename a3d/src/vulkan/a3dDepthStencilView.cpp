//-------------------------------------------------------------------------------------------------
// File : a3dDepthStencilView.cpp
// Desc : DepthStencilView Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DepthStencilView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DepthStencilView::DepthStencilView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pTexture    (nullptr)
, m_ImageView   (null_handle)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DepthStencilView::~DepthStencilView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool DepthStencilView::Init(IDevice* pDevice, ITexture* pTexture, const TargetViewDesc* pDesc)
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
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (pDesc->Format == RESOURCE_FORMAT_D24_UNORM_S8_UINT)
        { aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT; }

        VkImageViewCreateInfo info = {};
        info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext        = nullptr;
        info.flags        = 0;
        info.image        = m_pTexture->GetVulkanImage();
        info.viewType     = ToNativeImageViewType(pDesc->Dimension);
        info.format       = ToNativeFormat(pDesc->Format);
        info.components.r = ToNativeComponentSwizzle(pDesc->ComponentMapping.R);
        info.components.g = ToNativeComponentSwizzle(pDesc->ComponentMapping.G);
        info.components.b = ToNativeComponentSwizzle(pDesc->ComponentMapping.B);
        info.components.a = ToNativeComponentSwizzle(pDesc->ComponentMapping.A);
        info.subresourceRange.aspectMask     = aspectMask;
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
void DepthStencilView::Term()
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
void DepthStencilView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void DepthStencilView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t DepthStencilView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void DepthStencilView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      テクスチャビューの構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetViewDesc DepthStencilView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      テクスチャの構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TextureDesc DepthStencilView::GetTextureDesc() const
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
VkImage DepthStencilView::GetVulkanImage() const
{
    if ( m_pTexture == nullptr )
    { return null_handle; }

    return m_pTexture->GetVulkanImage(); 
}

//-------------------------------------------------------------------------------------------------
//      イメージビューを取得します.
//-------------------------------------------------------------------------------------------------
VkImageView DepthStencilView::GetVulkanImageView() const
{ return m_ImageView; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ITexture* DepthStencilView::GetResource() const
{ return m_pTexture; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool DepthStencilView::Create
(
    IDevice*                pDevice,
    ITexture*               pTexture,
    const TargetViewDesc*   pDesc,
    IDepthStencilView**     ppView
)
{
    if (pDevice == nullptr || pTexture == nullptr || pDesc == nullptr || ppView == nullptr)
    { return false; }

    auto instance = new DepthStencilView();
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

