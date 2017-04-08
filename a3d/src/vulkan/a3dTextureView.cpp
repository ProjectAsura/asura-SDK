//-------------------------------------------------------------------------------------------------
// File : a3dTextureView.cpp
// Desc : Texture View Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      イメージビュータイプに変換します.
//-------------------------------------------------------------------------------------------------
VkImageViewType ToNativeImageViewType(a3d::VIEW_DIMENSION value)
{
    static VkImageViewType table[] = {
        VK_IMAGE_VIEW_TYPE_1D,              // VIEW_DIMENSION_BUFFER
        VK_IMAGE_VIEW_TYPE_1D,              // VIEW_DIMENSION_TEXTURE1D
        VK_IMAGE_VIEW_TYPE_1D_ARRAY,        // VIEW_DIMENSION_TEXTURE1D_ARRAY
        VK_IMAGE_VIEW_TYPE_2D,              // VIEW_DIMENSION_TEXTURE2D
        VK_IMAGE_VIEW_TYPE_2D_ARRAY,        // VIEW_DIMENSION_TEXTURE2D_ARRAY
        VK_IMAGE_VIEW_TYPE_CUBE,            // VIEW_DIMENSION_CUBEMAP
        VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,      // VIEW_DIMENSION_CUBEMAP_ARRAY
        VK_IMAGE_VIEW_TYPE_3D               // VIEW_DIMENSION_TEXTURE3D
    };

    return table[value];
}

//-------------------------------------------------------------------------------------------------
//      コンポーネントスウィズルに変換します.
//-------------------------------------------------------------------------------------------------
VkComponentSwizzle ToNativeComponentSwizzle(a3d::TEXTURE_SWIZZLE value)
{
    static VkComponentSwizzle table[] = {
        VK_COMPONENT_SWIZZLE_R,     // SWIZZLE_R
        VK_COMPONENT_SWIZZLE_G,     // SWIZZLE_G
        VK_COMPONENT_SWIZZLE_B,     // SWIZZLE_B
        VK_COMPONENT_SWIZZLE_A,     // SWIZZLE_A
        VK_COMPONENT_SWIZZLE_ZERO,  // SWIZZLE_ZERO
        VK_COMPONENT_SWIZZLE_ONE    // SWIZZLE_ONE
    };

    return table[value];
}

} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TextureView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
TextureView::TextureView()
: m_RefCount        (1)
, m_pDevice         (nullptr)
, m_pTexture        (nullptr)
, m_ImageView       (null_handle)
, m_ImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
TextureView::~TextureView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool TextureView::Init(IDevice* pDevice, ITexture* pTexture, const TextureViewDesc* pDesc)
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
        switch( pDesc->TextureAspect )
        {
        case TEXTURE_ASPECT_COLOR:
            { m_ImageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT; }
            break;

        case TEXTURE_ASPECT_DEPTH:
            { m_ImageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT; }
            break;

        case TEXTURE_ASPECT_STENCIL:
            { m_ImageAspectFlags = VK_IMAGE_ASPECT_STENCIL_BIT; }
            break;
        }

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
        info.subresourceRange.aspectMask     = m_ImageAspectFlags;
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
void TextureView::Term()
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
void TextureView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void TextureView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t TextureView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void TextureView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      テクスチャビューの構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TextureViewDesc TextureView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      テクスチャの構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TextureDesc TextureView::GetTextureDesc() const
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
VkImage TextureView::GetVulkanImage() const
{
    if ( m_pTexture == nullptr )
    { return null_handle; }

    return m_pTexture->GetVulkanImage(); 
}

//-------------------------------------------------------------------------------------------------
//      イメージビューを取得します.
//-------------------------------------------------------------------------------------------------
VkImageView TextureView::GetVulkanImageView() const
{ return m_ImageView; }

//-------------------------------------------------------------------------------------------------
//      イメージアスペクトフラグを取得します.
//-------------------------------------------------------------------------------------------------
VkImageAspectFlags TextureView::GetVulkanImageAspectFlags() const
{ return m_ImageAspectFlags; }

//-------------------------------------------------------------------------------------------------
//      ステートを取得します.
//-------------------------------------------------------------------------------------------------
RESOURCE_STATE TextureView::GetState() const
{
    if (m_pTexture == nullptr)
    { return RESOURCE_STATE_UNKNOWN; }

    return m_pTexture->GetState();
}

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool TextureView::Create
(
    IDevice*                pDevice,
    ITexture*               pTexture,
    const TextureViewDesc*  pDesc,
    ITextureView**          ppTextureView
)
{
    if (pDevice == nullptr || pTexture == nullptr || pDesc == nullptr)
    { return false; }

    auto instance = new TextureView;
    if ( instance == nullptr )
    { return false; }

    if ( !instance->Init(pDevice, pTexture, pDesc) )
    {
        a3d::SafeRelease(instance);
        return false;
    }

    *ppTextureView = instance;
    return true;
}

} // namespace a3d

