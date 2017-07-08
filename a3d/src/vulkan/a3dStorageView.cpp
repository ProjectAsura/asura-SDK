//-------------------------------------------------------------------------------------------------
// File : a3dStorageView.cpp
// Desc : Storage View Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// StorageView class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
StorageView::StorageView()
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pResource   (nullptr)
, m_ImageView   (null_handle)
, m_Buffer      (null_handle)
{ memset(&m_Desc, 0, sizeof(m_Desc)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
StorageView::~StorageView()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool StorageView::Init(IDevice* pDevice, IResource* pResource, const StorageViewDesc* pDesc)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr)
    { return false; }

    Term();

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_pResource = pResource;
    m_pResource->AddRef();

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    if (pResource->GetKind() == RESOURCE_KIND_BUFFER)
    {
        auto pWrapBuffer = static_cast<Buffer*>(pResource);
        A3D_ASSERT(pWrapBuffer != nullptr);

        auto bufferDesc = pWrapBuffer->GetDesc();
        if ((bufferDesc.Usage & RESOURCE_USAGE_STORAGE_TARGET) != RESOURCE_USAGE_STORAGE_TARGET)
        { return false; }

        m_Buffer = pWrapBuffer->GetVulkanBuffer();
    }

    if (pResource->GetKind() == RESOURCE_KIND_TEXTURE)
    {
        auto pWrapTexture = static_cast<Texture*>(pResource);
        A3D_ASSERT(pWrapTexture != nullptr);

        auto textureDesc = pWrapTexture->GetDesc();
        if ((textureDesc.Usage & RESOURCE_USAGE_STORAGE_TARGET) != RESOURCE_USAGE_STORAGE_TARGET)
        { return false; }

        VkImageViewCreateInfo info = {};
        info.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext                              = nullptr;
        info.flags                              = 0;
        info.format                             = ToNativeFormat(pDesc->Format);
        info.image                              = pWrapTexture->GetVulkanImage();
        info.viewType                           = ToNativeImageViewType(pDesc->Dimension);
        info.components.r                       = VK_COMPONENT_SWIZZLE_R;
        info.components.g                       = VK_COMPONENT_SWIZZLE_G;
        info.components.b                       = VK_COMPONENT_SWIZZLE_B;
        info.components.a                       = VK_COMPONENT_SWIZZLE_A;
        info.subresourceRange.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel      = pDesc->MipSlice;
        info.subresourceRange.levelCount        = pDesc->MipLevels;
        info.subresourceRange.baseArrayLayer    = uint32_t(pDesc->FirstElements);
        info.subresourceRange.layerCount        = pDesc->ElementCount;

        auto ret = vkCreateImageView(pNativeDevice, &info, nullptr, &m_ImageView);
        if (ret != VK_SUCCESS)
        { return false; }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void StorageView::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pNativeDevice = m_pDevice->GetVulkanDevice();
    A3D_ASSERT( pNativeDevice != null_handle );

    if (m_ImageView != null_handle)
    {
        vkDestroyImageView( pNativeDevice, m_ImageView, nullptr );
        m_ImageView = null_handle;
    }

    SafeRelease( m_pResource );
    SafeRelease( m_pDevice );
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを増やします.
//-------------------------------------------------------------------------------------------------
void StorageView::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void StorageView::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウンタを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t StorageView::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void StorageView::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
StorageViewDesc StorageView::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      リソースを取得します.
//-------------------------------------------------------------------------------------------------
IResource* StorageView::GetResource() const
{ return m_pResource; }

//-------------------------------------------------------------------------------------------------
//      イメージビューを取得します.
//-------------------------------------------------------------------------------------------------
VkImageView StorageView::GetVulkanImageView() const
{ return m_ImageView; }

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
VkBuffer StorageView::GetVulkanBuffer() const
{ return m_Buffer; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool StorageView::Create
(
    IDevice*                pDevice,
    IResource*              pResource,
    const StorageViewDesc*  pDesc,
    IStorageView**          ppStorageView
)
{
    if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr || ppStorageView == nullptr)
    { return false; }

    auto instance = new StorageView();
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pResource, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppStorageView = instance;
    return true;
}

} // namespace a3d
