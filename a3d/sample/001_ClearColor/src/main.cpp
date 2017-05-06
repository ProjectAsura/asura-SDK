//-------------------------------------------------------------------------------------------------
// File : main.cpp
// Desc : Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3d.h>
#include <cstdlib>
#include <cassert>
#include <SampleApp.h>


//-------------------------------------------------------------------------------------------------
// Forward declarations.
//-------------------------------------------------------------------------------------------------
bool InitA3D();
void TermA3D();
void DrawA3D();
void Resize(uint32_t w, uint32_t h, void* ptr);

//-------------------------------------------------------------------------------------------------
// Global Varaibles.
//-------------------------------------------------------------------------------------------------
IApp*               g_pApp              = nullptr;  //!< �E�B���h�E�̐������s���w���p�[�N���X�ł�.
a3d::IDevice*       g_pDevice           = nullptr;  //!< �f�o�C�X�ł�.
a3d::ISwapChain*    g_pSwapChain        = nullptr;  //!< �X���b�v�`�F�C���ł�.
a3d::IQueue*        g_pGraphicsQueue    = nullptr;  //!< �R�}���h�L���[�ł�.
a3d::IFence*        g_pFence            = nullptr;  //!< �t�F���X�ł�.
a3d::ITexture*      g_pColorBuffer[2]   = {};       //!< �J���[�o�b�t�@�ł�.
a3d::ITextureView*  g_pColorView[2]     = {};       //!< �J���[�r���[�ł�.
a3d::ICommandList*  g_pCommandList[2]   = {};       //!< �R�}���h���X�g�ł�.
a3d::IFrameBuffer*  g_pFrameBuffer[2]   = {};       //!< �t���[���o�b�t�@�ł�.
bool                g_Prepare           = false;    //!< �����o������true.


///////////////////////////////////////////////////////////////////////////////////////////////////
// Allocator class
///////////////////////////////////////////////////////////////////////////////////////////////////
class Allocator : public a3d::IAllocator
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NONTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOHTING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //      ���������m�ۂ��܂�.
    //---------------------------------------------------------------------------------------------
    void* Alloc(size_t size, size_t alignment) noexcept override
    {
        #if A3D_IS_WIN
            return _aligned_malloc(size, alignment);
        #else
            return aligned_alloc(alignment, size);
        #endif
    }

    //---------------------------------------------------------------------------------------------
    //      ���������Ċm�ۂ��܂�.
    //---------------------------------------------------------------------------------------------
    void* Realloc(void* ptr, size_t size, size_t alignment) noexcept override
    {
        #if A3D_IS_WIN
            return _aligned_realloc(ptr, size, alignment);
        #else
            A3D_UNUSED(alignment);
            return realloc(ptr, size);
        #endif
    }

    //---------------------------------------------------------------------------------------------
    //      ��������������܂�.
    //---------------------------------------------------------------------------------------------
    void Free(void* ptr) noexcept override
    {
        #if A3D_IS_WIN
            _aligned_free(ptr);
        #else
            free(ptr);
        #endif
    }
} g_Allocator;


//-------------------------------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-------------------------------------------------------------------------------------------------
void Main()
{
    // �E�B���h�E����.
    if (!CreateApp(960, 540, &g_pApp))
    { return; }

    // ���T�C�Y���̃R�[���o�b�N�֐���ݒ�.
    g_pApp->SetResizeCallback(Resize, nullptr);

    // A3D������.
    if (!InitA3D())
    {
        g_pApp->Release();
        return;
    }

    // ���C�����[�v.
    while( g_pApp->IsLoop() )
    {
        // �`�揈��.
        DrawA3D();
    }

    // ��n��.
    TermA3D();
    g_pApp->Release();
}

//-------------------------------------------------------------------------------------------------
//      A3D�̏��������s���܂�.
//-------------------------------------------------------------------------------------------------
bool InitA3D()
{
    g_Prepare = false;

    // �O���t�B�b�N�X�V�X�e���̏�����.
    {
        a3d::SystemDesc desc = {};
        desc.pAllocator = &g_Allocator;
        desc.pOption    = g_pApp->GetWindowHandle();

        if (!a3d::InitSystem(&desc))
        { return false; }
    }

    // �f�o�C�X�̐���.
    {
        a3d::DeviceDesc desc = {};

        desc.EnableDebug = true;

        // �ő�f�B�X�N���v�^����ݒ�.
        desc.MaxColorTargetCount            = 2;
        desc.MaxDepthTargetCount            = 1;
        desc.MaxShaderResourceCount         = 1;

        // �ő�T�u�~�b�g����ݒ�.
        desc.MaxGraphicsQueueSubmitCount    = 256;
        desc.MaxCopyQueueSubmitCount        = 256;
        desc.MaxComputeQueueSubmitCount     = 256;

        // �f�o�C�X�𐶐�.
        if (!a3d::CreateDevice(&desc, &g_pDevice))
        { return false; }

        // �R�}���h�L���[���擾.
        g_pDevice->GetGraphicsQueue(&g_pGraphicsQueue);
    }

    // �X���b�v�`�F�C���̐���.
    {
    #if SAMPLE_IS_VULKAN && TARGET_PC
        auto format = a3d::RESOURCE_FORMAT_B8G8R8A8_UNORM;
    #else
        auto format = a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM;
    #endif

        a3d::SwapChainDesc desc = {};
        desc.Extent.Width   = g_pApp->GetWidth();
        desc.Extent.Height  = g_pApp->GetHeight();
        desc.Format         = format;
        desc.MipLevels      = 1;
        desc.SampleCount    = 1;
        desc.BufferCount    = 2;
        desc.SyncInterval   = 1;
        desc.InstanceHandle = g_pApp->GetInstanceHandle();
        desc.WindowHandle   = g_pApp->GetWindowHandle();

        if (!g_pDevice->CreateSwapChain(&desc, &g_pSwapChain))
        { return false; }

        // �X���b�v�`�F�C������o�b�t�@���擾.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

        a3d::TextureViewDesc viewDesc = {};
        viewDesc.Dimension          = a3d::VIEW_DIMENSION_TEXTURE2D;
        viewDesc.Format             = format;
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
            if (!g_pDevice->CreateTextureView(g_pColorBuffer[i], &viewDesc, &g_pColorView[i]))
            { return false; }
        }
    }

    // �t���[���o�b�t�@�̐���
    {
        // �t���[���o�b�t�@�̐ݒ�.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = g_pColorView[0];
        desc.pDepthTarget       = nullptr;

        // 1���ڂ̃t���[���o�b�t�@�𐶐�.
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[0]))
        { return false; }

        // 2���ڂ̃t���[���o�b�t�@�𐶐�.
        desc.pColorTargets[0] = g_pColorView[1];
        if (!g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[1]))
        { return false; }
    }

    // �R�}���h���X�g�𐶐�.
    {
        for(auto i=0; i<2; ++i)
        {
            if (!g_pDevice->CreateCommandList(a3d::COMMANDLIST_TYPE_DIRECT, &g_pCommandList[i]))
            { return false; }
        }
    }

    // �t�F���X�𐶐�.
    {
        if (!g_pDevice->CreateFence(&g_pFence))
        { return false; }
    }

    g_Prepare = true;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      A3D�̏I���������s���܂�.
//-------------------------------------------------------------------------------------------------
void TermA3D()
{
    g_Prepare = false;

    // �_�u���o�b�t�@���\�[�X�̔j��.
    for(auto i=0; i<2; ++i)
    {
        // �t���[���o�b�t�@�̔j��.
        a3d::SafeRelease(g_pFrameBuffer[i]);

        // �J���[�r���[�̔j��.
        a3d::SafeRelease(g_pColorView[i]);

        // �J���[�o�b�t�@�̔j��.
        a3d::SafeRelease(g_pColorBuffer[i]);

        // �R�}���h���X�g�̔j��.
        a3d::SafeRelease(g_pCommandList[i]);
    }

    // �t�F���X�̔j��.
    a3d::SafeRelease(g_pFence);

    // �X���b�v�`�F�C���̔j��.
    a3d::SafeRelease(g_pSwapChain);

    // �O���t�B�b�N�X�L���[�̔j��.
    a3d::SafeRelease(g_pGraphicsQueue);

    // �f�o�C�X�̔j��.
    a3d::SafeRelease(g_pDevice);

    // �O���t�B�b�N�X�V�X�e���̏I������.
    a3d::TermSystem();
}

//-------------------------------------------------------------------------------------------------
//      A3D�ɂ��`�揈�����s���܂�.
//-------------------------------------------------------------------------------------------------
void DrawA3D()
{
    if (!g_Prepare)
    { return; }

    // �o�b�t�@�ԍ����擾���܂�.
    auto idx = g_pSwapChain->GetCurrentBufferIndex();

    // �R�}���h�̋L�^���J�n���܂�.
    auto pCmd = g_pCommandList[idx];
    pCmd->Begin();

    // �������ݗp�̃o���A��ݒ肵�܂�.
    pCmd->TextureBarrier(
        g_pColorBuffer[idx],
        a3d::RESOURCE_STATE_PRESENT,
        a3d::RESOURCE_STATE_COLOR_WRITE);

    // �t���[���o�b�t�@��ݒ肵�܂�.
    pCmd->SetFrameBuffer(g_pFrameBuffer[idx]);

    // �t���[���o�b�t�@���N���A���܂�.
    a3d::ClearColorValue clearColor = {};
    clearColor.Float[0] = 0.25f;
    clearColor.Float[1] = 0.25f;
    clearColor.Float[2] = 0.25f;
    clearColor.Float[3] = 1.0f;
    pCmd->ClearFrameBuffer(1, &clearColor, nullptr);

    {
        /* TODO */
    }

    pCmd->SetFrameBuffer(nullptr);

    // �\���p�Ƀo���A��ݒ肵�܂�.
    pCmd->TextureBarrier(
        g_pColorBuffer[idx],
        a3d::RESOURCE_STATE_COLOR_WRITE,
        a3d::RESOURCE_STATE_PRESENT);

    // �R�}���h���X�g�ւ̋L�^���I�����܂�.
    pCmd->End();

    // �R�}���h�L���[�ɓo�^���܂�.
    g_pGraphicsQueue->Submit(pCmd);

    // �R�}���h�����s���܂�.
    g_pGraphicsQueue->Execute(g_pFence);

    // �R�}���h�����s������ҋ@���܂�.
    if (!g_pFence->IsSignaled())
    { g_pFence->Wait(UINT32_MAX); }

    // ��ʂɕ\�����܂�.
    g_pSwapChain->Present();
}

//-------------------------------------------------------------------------------------------------
//      ���T�C�Y�����ł�.
//-------------------------------------------------------------------------------------------------
void Resize( uint32_t w, uint32_t h, void* pUser )
{
    A3D_UNUSED( pUser );

    // �������������ĂȂ���Ԃ�������C�����ł��Ȃ��̂ő��I��.
    if (!g_Prepare || g_pSwapChain == nullptr)
    { return; }

    // �T���v�����ȉ��̏ꍇ�̓N���b�V�����錴���ƂȂ�̂ŏ��������Ȃ�.
    {
        auto desc = g_pSwapChain->GetDesc();
        if ( w < desc.SampleCount || h < desc.SampleCount )
        { return; }
    }

    g_Prepare = false;

    // �A�C�h����ԂɂȂ�܂ő҂�.
    g_pGraphicsQueue->WaitIdle();
    g_pDevice->WaitIdle();

    for(auto i=0; i<2; ++i)
    {
        // �t���[���o�b�t�@�̔j��.
        a3d::SafeRelease(g_pFrameBuffer[i]);

        // �J���[�r���[�̔j��.
        a3d::SafeRelease(g_pColorView[i]);

        // �J���[�o�b�t�@�̔j��.
        a3d::SafeRelease(g_pColorBuffer[i]);
    }

    // �X���b�v�`�F�C���̃��T�C�Y�����ł�.
    g_pSwapChain->ResizeBuffers( w, h );

    // �e�N�X�`���r���[�𐶐�.
    {
        auto desc = g_pSwapChain->GetDesc();

        // �X���b�v�`�F�C������o�b�t�@���擾.
        g_pSwapChain->GetBuffer(0, &g_pColorBuffer[0]);
        g_pSwapChain->GetBuffer(1, &g_pColorBuffer[1]);

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
            auto ret = g_pDevice->CreateTextureView(g_pColorBuffer[i], &viewDesc, &g_pColorView[i]);
            assert(ret == true);
        }
    }

    // �t���[���o�b�t�@�̐���
    {
        // �t���[���o�b�t�@�̐ݒ�.
        a3d::FrameBufferDesc desc = {};
        desc.ColorCount         = 1;
        desc.pColorTargets[0]   = g_pColorView[0];
        desc.pDepthTarget       = nullptr;

        // 1���ڂ̃t���[���o�b�t�@�𐶐�.
        auto ret = g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[0]);
        assert(ret == true);

        // 2���ڂ̃t���[���o�b�t�@�𐶐�.
        desc.pColorTargets[0] = g_pColorView[1];
        ret = g_pDevice->CreateFrameBuffer(&desc, &g_pFrameBuffer[1]);
        assert(ret == true);
    }

    g_Prepare = true;
}