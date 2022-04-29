//-------------------------------------------------------------------------------------------------
// File : a3dUnorderedAccessView.cpp
// Desc : Unordered Access View Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// UnorderedAccessView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
UnorderedAccessView::UnorderedAccessView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pResource   (nullptr)
, m_ImageView   (null_handle)
, m_Buffer      (null_handle)
{ memset(&m_Desc, 0, sizeof(m_Desc)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
UnorderedAccessView::~UnorderedAccessView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool UnorderedAccessView::Init(IDevice* pDevice, IResource* pResource, const UnorderedAccessViewDesc* pDesc)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_pResource = pResource;
    m_pResource->AddRef();

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if (pResource->GetKind() == RESOURCE_KIND_BUFFER)
    {
        auto pWrapBuffer = static_cast<Buffer*>(pResource);
        A3D_ASSERT(pWrapBuffer != nullptr);

        auto bufferDesc = pWrapBuffer->GetDesc();
        if ((bufferDesc.Usage & RESOURCE_USAGE_UNORDERED_ACCESS) != RESOURCE_USAGE_UNORDERED_ACCESS)
        {
            A3D_LOG("Error : Invalid Argument.");
            return false;
        }

        m_Buffer = pWrapBuffer->GetVkBuffer();
    }
    else if (pResource->GetKind() == RESOURCE_KIND_TEXTURE)
    {
        auto pWrapTexture = static_cast<Texture*>(pResource);
        A3D_ASSERT(pWrapTexture != nullptr);

        auto textureDesc = pWrapTexture->GetDesc();
        if ((textureDesc.Usage & RESOURCE_USAGE_UNORDERED_ACCESS) != RESOURCE_USAGE_UNORDERED_ACCESS)
        {
            A3D_LOG("Error : Invalid Argument.");
            return false;
        }

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        if (pDesc->Format == RESOURCE_FORMAT_D16_UNORM ||
            pDesc->Format == RESOURCE_FORMAT_D24_UNORM_S8_UINT ||
            pDesc->Format == RESOURCE_FORMAT_D32_FLOAT)
        {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        VkImageViewCreateInfo info = {};
        info.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext                              = nullptr;
        info.flags                              = 0;
        info.format                             = ToNativeFormat(pDesc->Format);
        info.image                              = pWrapTexture->GetVkImage();
        info.viewType                           = ToNativeImageViewType(pDesc->Dimension);
        info.components.r                       = VK_COMPONENT_SWIZZLE_R;
        info.components.g                       = VK_COMPONENT_SWIZZLE_G;
        info.components.b                       = VK_COMPONENT_SWIZZLE_B;
        info.components.a                       = VK_COMPONENT_SWIZZLE_A;
        info.subresourceRange.aspectMask        = aspectMask;
        info.subresourceRange.baseMipLevel      = pDesc->MipSlice;
        info.subresourceRange.levelCount        = pDesc->MipLevels;
        info.subresourceRange.baseArrayLayer    = uint32_t(pDesc->FirstElement);
        info.subresourceRange.layerCount        = pDesc->ElementCount;

        auto ret = vkCreateImageView(pNativeDevice, &info, nullptr, &m_ImageView);
        if (ret != VK_SUCCESS)
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
void UnorderedAccessView::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVkDevice();
    A3D_ASSERT( pNativeDevice != null_handle );

    if (m_ImageView != null_handle)
    {
        vkDestroyImageView( pNativeDevice, m_ImageView, nullptr );
        m_ImageView = null_handle;
    }

    m_Buffer = null_handle;
    SafeRelease( m_pResource );
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void UnorderedAccessView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void UnorderedAccessView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t UnorderedAccessView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void UnorderedAccessView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
UnorderedAccessViewDesc UnorderedAccessView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
IResource* UnorderedAccessView::GetResource() const
{ return m_pResource; }

//-------------------------------------------------------------------------------------------------
//      イメージビューを取得します.
//-------------------------------------------------------------------------------------------------
VkImageView UnorderedAccessView::GetVkImageView() const
{ return m_ImageView; }

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
VkBuffer UnorderedAccessView::GetVkBuffer() const
{ return m_Buffer; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool UnorderedAccessView::Create
(
    IDevice*                        pDevice,
    IResource*                      pResource,
    const UnorderedAccessViewDesc*  pDesc,
    IUnorderedAccessView**          ppView
)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr || ppView == nullptr)
    { return false; }

    auto instance = new UnorderedAccessView();
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pResource, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppView = instance;
    return true;
}

} // namespace a3d
