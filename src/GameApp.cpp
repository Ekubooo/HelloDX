#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight),
    m_IndexCount(),
    m_VSConstantBuffer(),
    m_PSConstantBuffer(),
    m_DirLight(),
    m_PointLight(),
    m_SpotLight(),
    m_IsWireframeMode(false)
{ 

}

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
    static float phi = 0.0f, theta = 0.0f;
    phi += 0.3f * dt, theta += 0.37f * dt;
    XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
    m_VSConstantBuffer.world = XMMatrixTranspose(W);
    m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

    if (ImGui::Begin("Lighting"))
    {
        static int curr_mesh_item = 0;
        const char* mesh_strs[] = 
        {
            "Box",
            "Sphere",
            "Cylinder",
            "Cone"
        };

        if (ImGui::Combo("Mesh", &curr_mesh_item, mesh_strs, ARRAYSIZE(mesh_strs)))
        {
            Geometry::MeshData<VertexPosNormalTex> meshData;
            switch (curr_mesh_item)
            {
            case 0: meshData = Geometry::CreateBox<VertexPosNormalTex>();         break;
            case 1: meshData = Geometry::CreateSphere<VertexPosNormalTex>();      break;
            case 2: meshData = Geometry::CreateCylinder<VertexPosNormalTex>();    break;
            case 3: meshData = Geometry::CreateCone<VertexPosNormalTex>();        break; 
            }
            ResetMesh(meshData);
        }

        bool mat_changed = false;
        ImGui::Text("Material");
        ImGui::PushID(3);
        ImGui::ColorEdit3("Ambient",  &m_PSConstantBuffer.material.ambient.x);
        ImGui::ColorEdit3("Diffuse",  &m_PSConstantBuffer.material.diffuse.x);
        ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.material.specular.x);
        ImGui::PopID();

        static int curr_light_item = 0;
        static const char* light_modes[] = 
        {
            "Directional Light",
            "Point Light",
            "Spot Light"
        };

        ImGui::Text("Light");

        if (ImGui::Combo("Light type", &curr_light_item, light_modes, ARRAYSIZE(light_modes)))
        {
            m_PSConstantBuffer.dirLight     = (curr_light_item == 0 ? m_DirLight : DirectionalLight());
            m_PSConstantBuffer.pointLight   = (curr_light_item == 1 ? m_PointLight : PointLight());
            m_PSConstantBuffer.spotLight    = (curr_light_item == 2 ? m_SpotLight : SpotLight());
        }

        bool light_changed = false;
        
        // controller id for distinguish componment
        ImGui::PushID(curr_light_item);
        if (curr_light_item == 0)
        {
            ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.dirLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.dirLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.dirLight.specular.x);
        }
        else if (curr_light_item == 1)
        {
            ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.pointLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.pointLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.pointLight.specular.x);
            ImGui::InputFloat("Range", &m_PSConstantBuffer.pointLight.range);
            ImGui::InputFloat3("Attenutation", &m_PSConstantBuffer.pointLight.att.x);
        }
        else
        {   
            ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.spotLight.ambient.x);
            ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.spotLight.diffuse.x);
            ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.spotLight.specular.x);
            ImGui::InputFloat("Spot", &m_PSConstantBuffer.spotLight.spot);
            ImGui::InputFloat("Range", &m_PSConstantBuffer.spotLight.range);
            ImGui::InputFloat3("Attenutation", &m_PSConstantBuffer.spotLight.att.x);
        }
        ImGui::PopID();


        if (ImGui::Checkbox("WiireFrame mode", &m_IsWireframeMode))
        {
            m_pd3dImmediateContext->RSSetState(m_IsWireframeMode ? m_pRSWireframe.Get() : nullptr);
        }
        ImGui::End();
        ImGui::Render();
        
        // update constant buffer, let cube rotate!
        D3D11_MAPPED_SUBRESOURCE mappedData;
        HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
        memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
        m_pd3dImmediateContext->Unmap(m_pConstantBuffers[0].Get(), 0);

        HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
        memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
        m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
        
    }
    
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    m_pd3dImmediateContext->ClearRenderTargetView
        (m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::SkyBlue));
    m_pd3dImmediateContext->ClearDepthStencilView
        (m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);

    // imgui will trigger Direct3D Draw.
    // need to bind backup_buffer on RP before here.
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;  // ??
    // Light shader ///////////////////////////////////
    // create vertex shader
    HR(CreateShaderFromFile(L"HLSL\\Light\\Light_VS.cso", L"HLSL\\Light\\Light_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

    // create and bind vertex layout
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout), 
       blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    // create fragment/pixel shader 
    HR(CreateShaderFromFile(L"HLSL\\Light\\Light_PS.cso", L"HLSL\\Light\\Light_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));   
    
    // Texture shader test ////////////////////////////
    // create vertex shader
    HR(CreateShaderFromFile(L"HLSL\\Light\\LightTex_VS.cso", L"HLSL\\Light\\LightTex_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

    // create and bind vertex layout
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout), 
       blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    // create fragment/pixel shader 
    HR(CreateShaderFromFile(L"HLSL\\Light\\LightTex_PS.cso", L"HLSL\\Light\\LightTex_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));   

    // 2D texture shader ///////////////////////////////
    /*  
    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_VS_2D.cso", L"HLSL\\Basic\\Basic_VS_2D.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader2D.GetAddressOf()));

    HR(m_pd3dDevice->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout2D.GetAddressOf()));

    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_PS_2D.cso", L"HLSL\\Basic\\Basic_PS_2D.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader2D.GetAddressOf()));
  
    // 3D texture shader ///////////////////////////////
    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_VS_3D.cso", L"HLSL\\Basic\\Basic_VS_3D.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader3D.GetAddressOf()));
    
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout3D.GetAddressOf()));

    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_PS_3D.cso", L"HLSL\\Basic\\Basic_PS_3D.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader3D.GetAddressOf()));
    */
    return true;
}

bool GameApp::InitResource()
{
    // init mesh model and setting into assemble phase
    auto meshData = Geometry::CreateBox<VertexPosNormalTex>();  // no template
    ResetMesh(meshData);

    // constant buffer setting /////////////////////////////////////
    // INIT: constant buffer description
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(VSConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // CREATE: constant buffer for vs and ps 
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
    cbd.ByteWidth = sizeof(PSConstantBuffer); 
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));
    
    // Texture and sampler /////////////////////////////////////////
    // INIT: texture 
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, m_pWoodCreate.GetAddressOf()));
    // HR(CreateWICTextureFromFile(m_pd3dDevice.Get(), L"Texture\\tex.png", nullptr, m_pTexPicture.GetAddressOf()));

    // INIT: tex sampler state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));


    // defualt light setting ///////////////////////////////////////
    // direction light 
    m_DirLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_DirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_DirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_DirLight.direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
    // point light
    m_PointLight.position = XMFLOAT3(0.0f, 0.0f, -10.0f);
    m_PointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_PointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_PointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
    m_PointLight.range = 25.0f;
    // spot light 
    m_SpotLight.position = XMFLOAT3(0.0f, 0.0f, -5.0f);
    m_SpotLight.direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
    m_SpotLight.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_SpotLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_SpotLight.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_SpotLight.att = XMFLOAT3(1.0f, 0.0f, 0.0f);
    m_SpotLight.spot = 12.0f;
    m_SpotLight.range = 10000.0f;

    // INIT: value of buffer
    m_VSConstantBuffer.world = XMMatrixIdentity();			
    m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f,  0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f,  0.0f, 0.0f)
    ));
    m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
    m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();

    m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_PSConstantBuffer.material.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);

    // using defualt direction Light
    m_PSConstantBuffer.dirLight = m_DirLight;

    // observe perspective position setting
    m_PSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);

    // update constant buffer for ps
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);

    // Initiative Rasterizer state /////////////////////////////////
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthClipEnable = true;
        // line frame mode enable, normal = nullptr
    HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));

    // input Assemble //////////////////////////////////////////////
    // Primitive Topology, input layout
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());

    // bind vertex shader to RP
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    
    // VS/PS const buffer, corresponding to const buffer in HLSL register b0/b1
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());

    // texure loader and sampler
    m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());
    m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pWoodCreate.GetAddressOf());
    // bing pixal shader to RP
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    // debug test setting //////////////////////////////////////////
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosNormalTexLayout");
    D3D11SetDebugObjectName(m_pConstantBuffers[0].Get(), "VSConstantBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffers[1].Get(), "PSConstantBuffer");

    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Light_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Light_PS");

    D3D11SetDebugObjectName(m_pSamplerState.Get(), "SSLinearWrap");

    return true;
}

template<class VertexType>
bool GameApp::ResetMesh(const Geometry::MeshData<VertexType> &meshData)
{
    // release old data
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();

    // vertex buffer setting ///////////////////////////////////////
    // INIT: vertex buffer description
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexType);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    // CREATE: vertex buffer
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = meshData.vertexVec.data();
    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.ReleaseAndGetAddressOf()));

    // input Assemble 
    UINT stride = sizeof(VertexType);
    UINT offset = 0;
    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

    // index buffer setting /////////////////////////////////////////
    // INIT: index buffer description
    m_IndexCount = (UINT)meshData.indexVec.size();
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = m_IndexCount * sizeof(DWORD);   // DWORD check
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    // CREATE: index buffer
    InitData.pSysMem = meshData.indexVec.data();
    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
    
    // input Assemble 
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // debug setting
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");

    return true;
}
