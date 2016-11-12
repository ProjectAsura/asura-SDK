//-------------------------------------------------------------------------------------------------
// File : a3dSampler.cpp
// Desc : Sampler Moudle.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      フィルタをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkFilter ToNativeFilter(a3d::FILTER_MODE mode)
{
    VkFilter table[] = {
        VK_FILTER_NEAREST,
        VK_FILTER_LINEAR
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      ミップマップモードをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkSamplerMipmapMode ToNativeMipmapMode(a3d::MIPMAP_MODE mode)
{
    VkSamplerMipmapMode table[] = {
        VK_SAMPLER_MIPMAP_MODE_NEAREST,
        VK_SAMPLER_MIPMAP_MODE_LINEAR
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      サンプラーアドレスモードをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkSamplerAddressMode ToNativeSamplerAddressMode(a3d::TEXTURE_ADDRESS_MODE mode)
{
    VkSamplerAddressMode table[] = {
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      ボーダーカラーをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
VkBorderColor ToNativeBorderColor(a3d::BORDER_COLOR color)
{
    VkBorderColor table[] = {
        VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
    };

    return table[color];
}

} // namespace /* anonymous */

namespace a3d {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Sampler::Sampler()
: m_RefCount(1)
, m_pDevice (nullptr)
, m_Sampler (null_handle)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Sampler::~Sampler()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool Sampler::Init(IDevice* pDevice, const SamplerDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    auto pWarpDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWarpDevice != nullptr);

    auto pNativeDevice = pWarpDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    VkSamplerCreateInfo info = {};
    info.sType                      = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.pNext                      = nullptr;
    info.flags                      = 0;
    info.magFilter                  = ToNativeFilter(pDesc->MagFilter);
    info.minFilter                  = ToNativeFilter(pDesc->MinFilter);
    info.mipmapMode                 = ToNativeMipmapMode(pDesc->MipMapMode);
    info.addressModeU               = ToNativeSamplerAddressMode(pDesc->AddressU);
    info.addressModeV               = ToNativeSamplerAddressMode(pDesc->AddressV);
    info.addressModeW               = ToNativeSamplerAddressMode(pDesc->AddressW);
    info.mipLodBias                 = pDesc->MipLodBias;
    info.anisotropyEnable           = pDesc->AnisotropyEnable;
    info.maxAnisotropy              = static_cast<float>(pDesc->MaxAnisotropy);
    info.compareEnable              = pDesc->CompareEnable;
    info.compareOp                  = ToNativeCompareOp(pDesc->CompareOp);
    info.minLod                     = pDesc->MinLod;
    info.maxLod                     = pDesc->MaxLod;
    info.borderColor                = ToNativeBorderColor(pDesc->BorderColor);
    info.unnormalizedCoordinates    = VK_FALSE;

    auto ret = vkCreateSampler(pNativeDevice, &info, nullptr, &m_Sampler);
    if ( ret != VK_SUCCESS )
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Sampler::Term()
{
    if (m_pDevice == nullptr)
    { return; }

    auto pWrapDevice = reinterpret_cast<Device*>(m_pDevice);
    A3D_ASSERT(pWrapDevice != nullptr);

    auto pNativeDevice = pWrapDevice->GetVulkanDevice();
    A3D_ASSERT(pNativeDevice != null_handle);

    vkDestroySampler(pNativeDevice, m_Sampler, nullptr);
    m_Sampler = null_handle;

    SafeRelease(m_pDevice);
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを増やします.
//-------------------------------------------------------------------------------------------------
void Sampler::AddRef()
{ m_RefCount++; }

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
void Sampler::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-------------------------------------------------------------------------------------------------
//      参照カウントを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t Sampler::GetCount() const
{ return m_RefCount; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
void Sampler::GetDevice(IDevice** ppDevice)
{
    *ppDevice = m_pDevice;
    if (m_pDevice != nullptr)
    { m_pDevice->AddRef(); }
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを取得します.
//-------------------------------------------------------------------------------------------------
VkSampler Sampler::GetVulkanSampler() const
{ return m_Sampler; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Sampler::Create(IDevice* pDevice, const SamplerDesc* pDesc, ISampler** ppSampler)
{
    if (pDevice == nullptr || pDesc == nullptr || ppSampler == nullptr)
    { return false; }

    auto instance = new Sampler;
    if (instance == nullptr)
    { return false; }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        return false;
    }

    *ppSampler = instance;
    return true;
}

} // namespace a3d
