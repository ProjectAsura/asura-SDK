//-------------------------------------------------------------------------------------------------
// File : a3dSampler.cpp
// Desc : Sampler Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      アドレスモードをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
D3D11_TEXTURE_ADDRESS_MODE ToNativeAddressMode( a3d::TEXTURE_ADDRESS_MODE mode )
{
    D3D11_TEXTURE_ADDRESS_MODE table[] = {
        D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_MIRROR,
        D3D11_TEXTURE_ADDRESS_CLAMP,
        D3D11_TEXTURE_ADDRESS_BORDER,
        D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
    };

    return table[mode];
}

//-------------------------------------------------------------------------------------------------
//      フィルタに変換します.
//-------------------------------------------------------------------------------------------------
D3D11_FILTER ToNativeFilter
(
    a3d::FILTER_MODE min,
    a3d::FILTER_MODE mag,
    a3d::MIPMAP_MODE mip,
    bool             compare,
    bool             anisotropy
)
{
    auto reduction = (compare) ? D3D11_FILTER_REDUCTION_TYPE_COMPARISON
                               : D3D11_FILTER_REDUCTION_TYPE_STANDARD;

    if ( anisotropy )
    { return D3D11_ENCODE_ANISOTROPIC_FILTER( reduction ); }

    return D3D11_ENCODE_BASIC_FILTER( min, mag, mip, reduction );
}

//-------------------------------------------------------------------------------------------------
//      ボーダーカラーをネイティブ形式に変換します.
//-------------------------------------------------------------------------------------------------
void ToNativeBorderColor( a3d::BORDER_COLOR color, FLOAT* pResult )
{
    switch( color )
    {
    case a3d::BORDER_COLOR_TRANSPARENT_BLACK:
        {
            pResult[0] = 0.0f;
            pResult[1] = 0.0f;
            pResult[2] = 0.0f;
            pResult[3] = 0.0f;
        }
        break;

    case a3d::BORDER_COLOR_OPAQUE_BLACK:
        {
            pResult[0] = 0.0f;
            pResult[1] = 0.0f;
            pResult[2] = 0.0f;
            pResult[3] = 1.0f;
        }
        break;

    case a3d::BORDER_COLOR_OPAQUE_WHITE:
        {
            pResult[0] = 1.0f;
            pResult[1] = 1.0f;
            pResult[2] = 1.0f;
            pResult[3] = 1.0f;
        }
        break;
    }
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
: m_RefCount    (1)
, m_pDevice     (nullptr)
, m_pSampler    (nullptr)
{ memset( &m_Desc, 0, sizeof(m_Desc) ); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Sampler::~Sampler()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期処理です.
//-------------------------------------------------------------------------------------------------
bool Sampler::Init(IDevice* pDevice, const SamplerDesc* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    m_pDevice = static_cast<Device*>(pDevice);
    m_pDevice->AddRef();

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    auto pNativeDevice = m_pDevice->GetD3D11Device();
    A3D_ASSERT( pNativeDevice != nullptr );

    {
        D3D11_SAMPLER_DESC desc = {};
        desc.Filter         = ToNativeFilter(
                                pDesc->MinFilter,
                                pDesc->MagFilter,
                                pDesc->MipMapMode,
                                pDesc->CompareEnable,
                                pDesc->AnisotropyEnable);
        desc.AddressU       = ToNativeAddressMode(pDesc->AddressU);
        desc.AddressV       = ToNativeAddressMode(pDesc->AddressV);
        desc.AddressW       = ToNativeAddressMode(pDesc->AddressW);
        desc.MipLODBias     = pDesc->MipLodBias;
        desc.MaxAnisotropy  = pDesc->MaxAnisotropy;
        desc.ComparisonFunc = ToNativeComparisonFunc(pDesc->CompareOp);
        desc.MinLOD         = pDesc->MinLod;
        desc.MaxLOD         = pDesc->MaxLod;
        ToNativeBorderColor(pDesc->BorderColor, desc.BorderColor);

        auto hr = pNativeDevice->CreateSamplerState(&desc, &m_pSampler);
        if ( FAILED(hr) )
        {
            A3D_LOG("Error : ID3D11Device::CreateSamplerState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void Sampler::Term()
{
    SafeRelease(m_pSampler);
    SafeRelease(m_pDevice);
    memset( &m_Desc, 0, sizeof(m_Desc) );
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
//      デバッグ名を設定します.
//-------------------------------------------------------------------------------------------------
void Sampler::SetName(const char* name)
{
    m_Name = name;
    m_pSampler->SetPrivateData(WKPDID_D3DDebugObjectName, m_Name.size(), m_Name.c_str());
}

//-------------------------------------------------------------------------------------------------
//      デバッグ名を取得します.
//-------------------------------------------------------------------------------------------------
const char* Sampler::GetName() const
{ return m_Name.c_str(); }

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
//      サンプラーステートを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11SamplerState* Sampler::GetD3D11SamplerState() const
{ return m_pSampler; }

//-------------------------------------------------------------------------------------------------
//      生成処理を行います.
//-------------------------------------------------------------------------------------------------
bool Sampler::Create(IDevice* pDevice, const SamplerDesc* pDesc, ISampler** ppSampler)
{
    if (pDevice == nullptr || pDesc == nullptr || ppSampler == nullptr)
    {
        A3D_LOG("Error : Invalid Argument.");
        return false;
    }

    auto instance = new Sampler();
    if (instance == nullptr)
    {
        A3D_LOG("Error : Out Of Memory.");
        return false;
    }

    if (!instance->Init(pDevice, pDesc))
    {
        SafeRelease(instance);
        A3D_LOG("Error : Init() Failed.");
        return false;
    }

    *ppSampler = instance;
    return true;
}

} // namespace a3d
