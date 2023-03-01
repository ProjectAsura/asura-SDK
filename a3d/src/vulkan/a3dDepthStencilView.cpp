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
    {
        A3D_LOG("Error : Invalid Argument");
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
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (pDesc->Format == RESOURCE_FORMAT_D24_UNORM_S8_UINT)
        { aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT; }

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
        info.subresourceRange.aspectMask     = aspectMask;
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
void DepthStencilView::Term()
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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void DepthStencilView::SetName(const char* name)
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
const char* DepthStencilView::GetName() const
{ return m_Name.c_str(); }

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
VkImage DepthStencilView::GetVkImage() const
{
    if ( m_pTexture == nullptr )
    { return null_handle; }

    return m_pTexture->GetVkImage(); 
}

//-------------------------------------------------------------------------------------------------
//      イメージビューを取得します.
//-------------------------------------------------------------------------------------------------
VkImageView DepthStencilView::GetVkImageView() const
{ return m_ImageView; }

//-------------------------------------------------------------------------------------------------
//      テクスチャビューの構成設定を取得します.
//-------------------------------------------------------------------------------------------------
TargetViewDesc IDepthStencilView::GetDesc() const
{
    auto pThis = static_cast<const DepthStencilView*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_Desc;
}

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
ITexture* IDepthStencilView::GetResource() const
{
    auto pThis = static_cast<const DepthStencilView*>(this);
    A3D_ASSERT(pThis != nullptr);

    return pThis->m_pTexture;
}

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
    {
        A3D_LOG("Error : Invalid Argument");
        return false;
    }

    auto instance = new DepthStencilView();
    if ( instance == nullptr )
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if ( !instance->Init(pDevice, pTexture, pDesc) )
    {
        a3d::SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppView = instance;
    return true;
}

} // namespace a3d

