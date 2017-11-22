//-------------------------------------------------------------------------------------------------
// File : app_framework.h
// Desc : Framework
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <app_framework.h>
#include <app_allocator.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// Framework class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      �R���X�g���N�^.
//-------------------------------------------------------------------------------------------------
Framework::Framework()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      �f�X�g���N�^.
//-------------------------------------------------------------------------------------------------
Framework::~Framework()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      ���s.
//-------------------------------------------------------------------------------------------------
void Framework::Run
(
    uint32_t                width,
    uint32_t                height,
    a3d::RESOURCE_FORMAT    colorFormat,
    a3d::RESOURCE_FORMAT    depthFormat
)
{
    if (Init(width, height, colorFormat, depthFormat))
    { MainLoop(); }

    Term();
}

//-------------------------------------------------------------------------------------------------
//      ���������̏���.
//-------------------------------------------------------------------------------------------------
bool Framework::Init
(
    uint32_t                width,
    uint32_t                height,
    a3d::RESOURCE_FORMAT    colorFormat,
    a3d::RESOURCE_FORMAT    depthFormat
)
{
    m_Ready = false;

    // �A�v���̐���.
    {
        if (!a3d::CreateApp(Allocator::Instance(), width, height, this, this, this, this, &m_pApp))
        { return false; }
    }

    // �O���t�B�b�N�X�V�X�e���̏�����.
    {
        a3d::SystemDesc desc = {};
        desc.pAllocator = Allocator::Instance();
        desc.pOption    = m_pApp->GetWindowHandle();

        if (!a3d::InitSystem(&desc))
        { return false; }
    }

    // �f�o�C�X�̐���.
    {
        a3d::DeviceDesc desc = {};

        desc.EnableDebug = true;

        // �ő�f�B�X�N���v�^����ݒ�.
        desc.MaxColorTargetCount            = 512;
        desc.MaxDepthTargetCount            = 512;
        desc.MaxShaderResourceCount         = 4096;

        // �ő�T�u�~�b�g����ݒ�.
        desc.MaxGraphicsQueueSubmitCount    = 256;
        desc.MaxCopyQueueSubmitCount        = 256;
        desc.MaxComputeQueueSubmitCount     = 256;

        // �f�o�C�X�𐶐�.
        if (!a3d::CreateDevice(&desc, &m_pDevice))
        { return false; }

        // �R�}���h�L���[���擾.
        m_pDevice->GetGraphicsQueue(&m_pGraphicsQueue);
    }

    // �X���b�v�`�F�C���̐���.
    {
        a3d::SwapChainDesc desc = {};
        desc.Extent.Width   = m_pApp->GetWidth();
        desc.Extent.Height  = m_pApp->GetHeight();
        desc.Format         = colorFormat;
        desc.MipLevels      = 1;
        desc.SampleCount    = 1;
        desc.BufferCount    = 2;
        desc.SyncInterval   = 1;
        desc.InstanceHandle = m_pApp->GetInstanceHandle();
        desc.WindowHandle   = m_pApp->GetWindowHandle();

        if (!m_pDevice->CreateSwapChain(&desc, &m_pSwapChain))
        { return false; }

        // �X���b�v�`�F�C������o�b�t�@���擾.
        m_pSwapChain->GetBuffer(0, &m_pColorBuffer[0]);
        m_pSwapChain->GetBuffer(1, &m_pColorBuffer[1]);

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = colorFormat;
        viewDesc.TextureAspect      = a3d::TEXTURE_ASPECT_COLOR;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = 1;
        viewDesc.ComponentMapping.R = a3d::TEXTURE_SWIZZLE_R;
        viewDesc.ComponentMapping.G = a3d::TEXTURE_SWIZZLE_G;
        viewDesc.ComponentMapping.B = a3d::TEXTURE_SWIZZLE_B;
        viewDesc.ComponentMapping.A = a3d::TEXTURE_SWIZZLE_A;

        for(auto i=0; i<2; ++i)
        {
            if (!m_pDevice->CreateTextureView(m_pColorBuffer[i], &viewDesc, &m_pColorView[i]))
            { return false; }
        }
    }

    // �[�x�o�b�t�@�̐���.
    {
        a3d::TextureDesc desc = {};
        desc.Dimension                      = a3d::RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width                          = m_pApp->GetWidth();
        desc.Height                         = m_pApp->GetHeight();
        desc.DepthOrArraySize               = 1;
        desc.Format                         = depthFormat;
        desc.MipLevels                      = 1;
        desc.SampleCount                    = 1;
        desc.Layout                         = a3d::RESOURCE_LAYOUT_OPTIMAL;
        desc.Usage                          = a3d::RESOURCE_USAGE_DEPTH_TARGET;
        desc.InitState                      = a3d::RESOURCE_STATE_DEPTH_WRITE;
        desc.HeapProperty.Type              = a3d::HEAP_TYPE_DEFAULT;
        desc.HeapProperty.CpuPageProperty   = a3d::CPU_PAGE_PROPERTY_DEFAULT;

        if (!m_pDevice->CreateTexture(&desc, &m_pDepthBuffer))
        { return false; }

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = desc.Format;
        viewDesc.TextureAspect      = a3d::TEXTURE_ASPECT_DEPTH;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = desc.DepthOrArraySize;
        viewDesc.ComponentMapping.R = a3d::TEXTURE_SWIZZLE_R;
        viewDesc.ComponentMapping.G = a3d::TEXTURE_SWIZZLE_G;
        viewDesc.ComponentMapping.B = a3d::TEXTURE_SWIZZLE_B;
        viewDesc.ComponentMapping.A = a3d::TEXTURE_SWIZZLE_A;

        if (!m_pDevice->CreateTextureView(m_pDepthBuffer, &viewDesc, &m_pDepthView))
        { return false; }
    }

    // �t���[���o�b�t�@�̐���
    {
        // �t���[���o�b�t�@�̐ݒ�.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = m_pColorView[0];
        desc.pDepthTarget       = m_pDepthView;

        // 1���ڂ̃t���[���o�b�t�@�𐶐�.
        if (!m_pDevice->CreateFrameBuffer(&desc, &m_pFrameBuffer[0]))
        { return false; }

        // 2���ڂ̃t���[���o�b�t�@�𐶐�.
        desc.pColorTargets[0] = m_pColorView[1];
        if (!m_pDevice->CreateFrameBuffer(&desc, &m_pFrameBuffer[1]))
        { return false; }
    }

    // �R�}���h���X�g�𐶐�.
    {
        for(auto i=0; i<2; ++i)
        {
            if (!m_pDevice->CreateCommandList(a3d::COMMANDLIST_TYPE_DIRECT, &m_pCommandList[i]))
            { return false; }
        }
    }

    // �t�F���X�𐶐�.
    {
        if (!m_pDevice->CreateFence(&m_pFence))
        { return false; }
    }

    // GUI�����_���[������.
    {
        a3d::TargetFormat rtvFormat;
        rtvFormat.Format        = colorFormat;
        rtvFormat.SampleCount   = 1;

        a3d::TargetFormat dsvFormat;
        dsvFormat.Format        = depthFormat;
        dsvFormat.SampleCount   = 1;

        if (!a3d::GuiRenderer::Instance().Init(
            m_pDevice,
            m_pApp->GetWindowHandle(),
            m_pApp->GetWidth(),
            m_pApp->GetHeight(),
            rtvFormat,
            dsvFormat,
            nullptr))
        { return false; }
    }

    m_Ready = true;

    // ���[�U�[��`�̏������������Ăяo��.
    if (!OnInit())
    { return false; }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      ���C�����[�v.
//-------------------------------------------------------------------------------------------------
void Framework::MainLoop()
{
    while(m_pApp->IsLoop())
    {
        // �`�揈��.
        Draw();

        // ����Ȃ����\�[�X��j��.
        FrameDispose();
    }
}

//-------------------------------------------------------------------------------------------------
//      �I�����̏���.
//-------------------------------------------------------------------------------------------------
void Framework::Term()
{
    if (m_pGraphicsQueue != nullptr)
    { m_pGraphicsQueue->WaitIdle(); }

    m_Ready = false;

    OnTerm();

    a3d::GuiRenderer::Instance().Term();

    {
        auto itr = std::begin(m_Disposer);
        while(itr != std::end(m_Disposer))
        {
            a3d::SafeRelease(itr->pItem);
            itr++;
        }
        m_Disposer.clear();
    }

    for(auto i=0; i<2; ++i)
    {
        a3d::SafeRelease(m_pFrameBuffer[i]);
        a3d::SafeRelease(m_pColorView[i]);
        a3d::SafeRelease(m_pColorBuffer[i]);
        a3d::SafeRelease(m_pCommandList[i]);
    }

    a3d::SafeRelease(m_pDepthView);
    a3d::SafeRelease(m_pDepthBuffer);
    a3d::SafeRelease(m_pFence);
    a3d::SafeRelease(m_pSwapChain);
    a3d::SafeRelease(m_pGraphicsQueue);
    a3d::SafeRelease(m_pDevice);
    a3d::TermSystem();
}

//-------------------------------------------------------------------------------------------------
//      �`�揈��.
//-------------------------------------------------------------------------------------------------
void Framework::Draw()
{
    if (!m_Ready || m_pSwapChain == nullptr)
    { return; }

    OnDraw();

    if (m_ChangeMode)
    {
        // �A�C�h����ԂɂȂ�܂ő҂�.
        m_pGraphicsQueue->WaitIdle();
        m_pDevice->WaitIdle();

        // �t���X�N���[���؂�ւ�.
        m_pSwapChain->SetFullScreenMode(m_FullScreen);
        m_ChangeMode = false;
    }
}

//-------------------------------------------------------------------------------------------------
//      �s�v���\�[�X�̔j��.
//-------------------------------------------------------------------------------------------------
void Framework::FrameDispose()
{
    auto itr = std::begin(m_Disposer);
    while(itr != std::end(m_Disposer))
    {
        if (itr->life == 0)
        {
            a3d::SafeRelease(itr->pItem);
            itr = m_Disposer.erase(itr);
        }
        else
        {
            itr->life--;
            itr++;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      ���T�C�Y����.
//-------------------------------------------------------------------------------------------------
void Framework::OnEvent(const a3d::ResizeEventArg& arg)
{
    // �������o���Ă��Ȃ��ꍇ�͂��̂܂܏I��.
    if (!m_Ready || m_pSwapChain == nullptr)
    { return; }

    // �T���v�����������ꍇ�̓N���b�V������̂ŏ��������Ȃ�.
    {
        auto desc = m_pSwapChain->GetDesc();
        if (arg.Width < desc.SampleCount || arg.Height < desc.SampleCount)
        { return; }
    }

    // ������ԂɈڍs.
    m_Ready = false;

    // �A�C�h����ԂɂȂ�܂ő҂�.
    m_pGraphicsQueue->WaitIdle();
    m_pDevice->WaitIdle();

    for(auto i=0; i<2; ++i)
    {
        // �t���[���o�b�t�@�̔j��.
        a3d::SafeRelease(m_pFrameBuffer[i]);

        // �J���[�r���[�̔j��.
        a3d::SafeRelease(m_pColorView[i]);

        // �J���[�o�b�t�@�̔j��.
        a3d::SafeRelease(m_pColorBuffer[i]);
    }

    // �X���b�v�`�F�C���̃��T�C�Y�����ł�.
    m_pSwapChain->ResizeBuffers( arg.Width, arg.Height );

    // �e�N�X�`���r���[�𐶐�.
    {
        auto desc = m_pSwapChain->GetDesc();

        // �X���b�v�`�F�C������o�b�t�@���擾.
        m_pSwapChain->GetBuffer(0, &m_pColorBuffer[0]);
        m_pSwapChain->GetBuffer(1, &m_pColorBuffer[1]);

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = desc.Format;
        viewDesc.TextureAspect      = a3d::TEXTURE_ASPECT_COLOR;
        viewDesc.MipSlice           = 0;
        viewDesc.MipLevels          = desc.MipLevels;
        viewDesc.FirstArraySlice    = 0;
        viewDesc.ArraySize          = 1;
        viewDesc.ComponentMapping.R = a3d::TEXTURE_SWIZZLE_R;
        viewDesc.ComponentMapping.G = a3d::TEXTURE_SWIZZLE_G;
        viewDesc.ComponentMapping.B = a3d::TEXTURE_SWIZZLE_B;
        viewDesc.ComponentMapping.A = a3d::TEXTURE_SWIZZLE_A;

        for(auto i=0; i<2; ++i)
        {
            auto ret = m_pDevice->CreateTextureView(m_pColorBuffer[i], &viewDesc, &m_pColorView[i]);
            assert(ret == true);
        }
    }

    // �t���[���o�b�t�@�̐���
    {
        // �t���[���o�b�t�@�̐ݒ�.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = m_pColorView[0];
        desc.pDepthTarget       = nullptr;

        // 1���ڂ̃t���[���o�b�t�@�𐶐�.
        auto ret = m_pDevice->CreateFrameBuffer(&desc, &m_pFrameBuffer[0]);
        assert(ret == true);

        // 2���ڂ̃t���[���o�b�t�@�𐶐�.
        desc.pColorTargets[0] = m_pColorView[1];
        ret = m_pDevice->CreateFrameBuffer(&desc, &m_pFrameBuffer[1]);
        assert(ret == true);
    }

    // ��������.
    m_Ready = true;

    // ���[�U�[��`�֐����Ăяo��.
    OnResize(arg);
}

//-------------------------------------------------------------------------------------------------
//      �}�E�X�̏���.
//-------------------------------------------------------------------------------------------------
void Framework::OnEvent(const a3d::MouseEventArg& arg)
{
    OnMouse(arg);
}

//-------------------------------------------------------------------------------------------------
//      �L�[�̏���.
//-------------------------------------------------------------------------------------------------
void Framework::OnEvent(const a3d::KeyEventArg& arg)
{
    OnKey(arg);
}

//-------------------------------------------------------------------------------------------------
//      �^�C�s���O����.
//-------------------------------------------------------------------------------------------------
void Framework::OnEvent(uint32_t keyCode)
{
    OnTyping(keyCode);
}

a3d::ICommandList* Framework::GetCurrentCommandList() const
{
    auto idx = m_pSwapChain->GetCurrentBufferIndex();
    return m_pCommandList[idx];
}

a3d::IFrameBuffer* Framework::GetCurrentFrameBuffer() const
{
    auto idx = m_pSwapChain->GetCurrentBufferIndex();
    return m_pFrameBuffer[idx];
}

a3d::ITexture* Framework::GetCurrentColorBuffer() const
{
    auto idx = m_pSwapChain->GetCurrentBufferIndex();
    return m_pColorBuffer[idx];
}

a3d::ITextureView* Framework::GetCurrentColorView() const
{
    auto idx = m_pSwapChain->GetCurrentBufferIndex();
    return m_pColorView[idx];
}

a3d::IApp* Framework::GetApp() const
{ return m_pApp; }

bool Framework::IsReady() const
{ return m_Ready; }

void Framework::AddToDisposer(a3d::IReference* pItem, uint32_t life)
{
    DisposeItem entry;
    entry.pItem = pItem;
    entry.life  = life;
    m_Disposer.push_back(entry);
}

void Framework::ToggleFullScreen()
{
    m_FullScreen = true;
    m_ChangeMode = true;
}
