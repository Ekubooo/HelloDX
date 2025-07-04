#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

// static member init 
// {SemanticName, SemanticIndex, Format, InputSlot, ByteOffset, SlotClass, ???}
const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = 
{   
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0} 
};  // init the layout of shader's structure, almost same as opengl's method

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight) { }

GameApp::~GameApp() 
{

}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;
    if (!InitEffect())
        return false;
    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
    // imgui example component
    // ImGui::ShowAboutWindow();
    // ImGui::ShowDemoWindow();
    // ImGui::ShowUserGuide(); 
    
    // get IO event
    ImGuiIO& io = ImGui::GetIO();

    // custom window and operation
    static float tx = 0.0f, ty = 0.0f, phi = 0.0f, 
        theta = 0.0f, scale = 1.0f, fov = XM_PIDIV2;
    static bool animateCube = true, customColor = false;
    if (animateCube)
    {
        phi += 0.3f * dt;
        theta += 0.37f * dt;
        phi = XMScalarModAngle(phi);
        theta = XMScalarModAngle(theta);
    }

    // imgui start  ///////////////////////////////
    if (ImGui::Begin("Using ImGui"))
    {
        ImGui::Checkbox("animate cube", &animateCube);   // check box
        // next component at 25 pxiel far in right side of same line
        ImGui::SameLine(0.0f, 25.0f);   
        if (ImGui::Button("reset params"))
        {
            tx = ty = phi = theta = 0.0f;
            scale = 1.0f;
            fov = XM_PIDIV2;
        }
        // 
        ImGui::Text("phi: %.2f degrees", XMConvertToDegrees(phi));
        ImGui::SliderFloat("##1", &phi, -XM_PI, XM_PI, "");
        ImGui::Text("theta: %.2f degrees", XMConvertToDegrees(theta));
        ImGui::SliderFloat("##2", &theta, -XM_PI, XM_PI, "");

        ImGui::Text("Position: (%.1f, %.1f, 0.0)", tx, ty);

        ImGui::Text("FOV: %.2f degrees", XMConvertToDegrees(fov));
        ImGui::SliderFloat("##3", &fov, XM_PIDIV4, XM_PI / 3 * 2, "");

        if (ImGui::Checkbox("using custom color", &customColor))
        {
            m_cBuffer.useCustomColor = customColor;
        }

        if (customColor)
        {
            ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&m_cBuffer.color));
        }
    }

    ImGui::End();
    // imgui end //////////////////////////////////
    
    // update constant buffer
    m_cBuffer.world = XMMatrixTranspose(
        XMMatrixScalingFromVector(XMVectorReplicate(scale)) * 
        XMMatrixRotationX(phi) * XMMatrixRotationY(theta) *
        XMMatrixTranslation(tx, ty, 0.0f));
    m_cBuffer.proj = XMMatrixTranspose
        (XMMatrixPerspectiveFovLH(fov, AspectRatio(), 1.0f, 1000.0f));

    // update constant buffer, let cube rotate!
    D3D11_MAPPED_SUBRESOURCE mppedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mppedData));
    memcpy_s(mppedData.pData, sizeof(m_cBuffer), &m_cBuffer, sizeof(m_cBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  
    m_pd3dImmediateContext->ClearRenderTargetView
        (m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&black));
    m_pd3dImmediateContext->ClearDepthStencilView
        (m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // draw triangle
    // m_pd3dImmediateContext->Draw(3, 0);
    m_pd3dImmediateContext->DrawIndexed(36, 0, 0);

    // imgui start-up
    ImGui::Render();

    // imgui will trigger Direct3D Draw.
    // need to bind backup_buffer on RP before here.
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;  // ??
    // create vertex shader
    HR(CreateShaderFromFile(L"HLSL\\Cube_VS.cso", L"HLSL\\Cube_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

    // create and bind layout
    HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout), 
       blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    // create fragment/pixel shader 
    HR(CreateShaderFromFile(L"HLSL\\Cube_PS.cso", L"HLSL\\Cube_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));   
    
    return true;
}

bool GameApp::InitResource()
{
    // Three triangle verteices
    // order in which the three vertices are given should be arranged clockwise
    VertexPosColor triangleVertices[] = 
    {
        { XMFLOAT3(0.0f, 0.5f, 0.5f),   XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(0.5f, -0.5f, 0.5f),  XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
    };
    
    // VertexPosColor vertices[] =
    VertexPosColor vertices[] =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f),   XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f),  XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }
    };

    // Vertex buffer description
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof vertices;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    // appoint init data
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData,sizeof(InitData));
    InitData.pSysMem = vertices;

    // create vertex buffer
    // {Buffer description, SubResource, Buffer}
    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    // index buffer /////////////////////////////////////////
    DWORD indices[] = 
    {
        // front
        0, 1, 2, 2, 3, 0,
        // left 
        4, 5, 1, 1, 0, 4,
        // top 
        1, 5, 6, 6, 2, 1,
        // back 
        7, 6, 5, 5, 4, 7,
        // right 
        3, 2, 6, 6, 7, 3,
        // bottom 
        4, 0, 3, 3, 7, 4
    }; 

    // INIT: index buffer description 
    D3D11_BUFFER_DESC ibd;  
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof indices;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    // CREATE: index buffer
    InitData.pSysMem = indices;
    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
    //input Assemble setting for index buffer
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // constant buffer /////////////////////////////////////
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // crerate constant buffer without init data
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));

    // init constant buffer data ///////////////////////////
    m_cBuffer.world = XMMatrixIdentity();
    m_cBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    ));
    m_cBuffer.proj = XMMatrixTranspose(
        XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
    // TODO: here

    // input Assemble //////////////////////////////////////
    // setting vertex buffer 
    UINT stride = sizeof(VertexPosColor);
    UINT offset = 0;
    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

    // setting the Primitive type and input layout
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());

    // bind shader to RP
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
        // bind the constant buffer that already update to the VS
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get() , nullptr, 0);

    // setting debug object name 
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Trangle_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Trangle_PS");

    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Cube_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Cube_PS");

    return true;
}



