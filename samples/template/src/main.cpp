//-------------------------------------------------------------------------------------------------
// File : main.cpp
// Desc : Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <app_framework.h>
#include <app_allocator.h>


class Sample : public Framework
{
private:
    void OnMouse (const a3d::MouseEventArg&) override; 
    void OnKey   (const a3d::KeyEventArg&) override;
    void OnResize(const a3d::ResizeEventArg&) override;
    void OnTyping(uint32_t keyCode) override;
    bool OnInit  () override;
    void OnTerm  () override;
    void OnDraw  () override;
};

//-------------------------------------------------------------------------------------------------
//      ÉÅÉCÉìä÷êîÇ≈Ç∑.
//-------------------------------------------------------------------------------------------------
void main()
{
    Sample sample;
    sample.Run(960, 540, a3d::RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB, a3d::RESOURCE_FORMAT_D32_FLOAT);
}

bool Sample::OnInit()
{
    return true;
}

void Sample::OnTerm()
{
}

void Sample::OnDraw()
{
}

void Sample::OnMouse(const a3d::MouseEventArg& arg)
{
}

void Sample::OnKey(const a3d::KeyEventArg& arg)
{
}

void Sample::OnResize(const a3d::ResizeEventArg& arg)
{
}

void Sample::OnTyping(uint32_t keyCode)
{
}


