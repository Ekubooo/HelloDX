#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = 
{
    // {SemanticName, SemanticIndex, Format, InputSlot, ByteOffset, SlotClass, ???}
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};  // init the layout of shader's structure, almost same as opengl's method

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight) { }

GameApp::~GameApp() { }

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    return true;
}

bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;  // ??
    // create vertex shader
    HR(CreateShaderFromFile(L"HLSL\\Triangle_VS.cso", L"HLSL\\Triangle_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

    // create and bind layout

    // create fragment/pixel shader 
    
}

bool GameApp::InitResource()
{

}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };  // RGBA = (0,0,255,255)
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), blue);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    HR(m_pSwapChain->Present(0, 0));
}
