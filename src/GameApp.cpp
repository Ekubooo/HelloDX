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
            Geometry::MeshData<VertexPosNormalColor> meshData;
            switch (curr_mesh_item)
            {
            case 0: meshData = Geometry::CreateBox<VertexPosNormalColor>();         break;
            case 1: meshData = Geometry::CreateSphere<VertexPosNormalColor>();      break;
            case 2: meshData = Geometry::CreateCylinder<VertexPosNormalColor>();    break;
            case 3: meshData = Geometry::CreateCone<VertexPosNormalColor>();        break; 
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
        (m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
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
    // create vertex shader
    HR(CreateShaderFromFile(L"HLSL\\Light_VS.cso", L"HLSL\\Light_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

    // create and bind vertex layout
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout), 
       blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    // create fragment/pixel shader 
    HR(CreateShaderFromFile(L"HLSL\\Light_PS.cso", L"HLSL\\Light_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));   
    
    return true;
}

bool GameApp::InitResource()
{
    // init mesh model
    auto meshData = Geometry::CreateBox<VertexPosNormalColor>();
    ResetMesh(meshData);

    // constant buffer setting /////////////////////////////////////
    // INIT: constant buffer description
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(VSConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd. CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // CREATE: constant buffer for vs and ps 
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
    cbd.ByteWidth = sizeof(PSConstantBuffer); 
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));
    
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

    // observe perspective setting
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
    rasterizerDesc. DepthClipEnable = true;
        // line frame mode enable, normal = nullptr
    HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));

    // input Assemble //////////////////////////////////////////////
    // Primitive Topology, input layout
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    // bind shader to RP
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    // VS constant buffer, corresponding to const buffer in HLSL register b0
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
    // PS constant buffer, corresponding to const buffer in HLSL register b1
    m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    // debug test setting //////////////////////////////////////////
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosNormalTexLayout");
    D3D11SetDebugObjectName(m_pConstantBuffers[0].Get(), "VSConstantBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffers[1].Get(), "PSConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Light_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Light_PS");

    return true;
}

bool GameApp::ResetMesh(const Geometry::MeshData<VertexPosNormalColor> &meshData)
{
    // release old data
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();

    // vertex buffer setting ///////////////////////////////////////
    // INIT: vertex buffer description
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexPosNormalColor);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    // CREATE: vertex buffer
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = meshData.vertexVec.data();
    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    // input Assemble 
    UINT stride = sizeof(VertexPosNormalColor);
    UINT offset = 0;
    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

    // index buffer setting /////////////////////////////////////////
    // INIT: index buffer description
    m_IndexCount = (UINT)meshData.indexVec.size();
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = m_IndexCount * sizeof(DWORD);   // ??
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    // CREATE: index buffer
    InitData.pSysMem = meshData.indexVec.data();
    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
    
    // input Assemble 
    m_pd3dImmediateContext->IASetIndexBuffer
        (m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // debug setting
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");

    return true;
}


void UpdateScenePrivious(float dt)
{
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
        ImGui::SliderFloat("Scale", &scale, 0.2f, 2.0f);

        ImGui::Text("phi: %.2f degrees", XMConvertToDegrees(phi));
        ImGui::SliderFloat("##1", &phi, -XM_PI, XM_PI, "");
        ImGui::Text("theta: %.2f degrees", XMConvertToDegrees(theta));
        ImGui::SliderFloat("##2", &theta, -XM_PI, XM_PI, "");

        ImGui::Text("Position: (%.1f, %.1f, 0.0)", tx, ty);

        ImGui::Text("FOV: %.2f degrees", XMConvertToDegrees(fov));
        ImGui::SliderFloat("##3", &fov, XM_PIDIV4, XM_PI / 3 * 2, "");

        if (ImGui::Checkbox("using custom color", &customColor))
        {
            // m_cBuffer.useCustomColor = customColor;
        }

        if (customColor)
        {
            // ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&m_cBuffer.color));
        }
    }

    ImGui::End();
    // ImGui end ///////////////////////////////////

    // device control event ////////////////////////
    if (!ImGui::IsAnyItemActive())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {   // mouse event moving (left click)
            tx += io.MouseDelta.x * 0.01f;
            ty -= io.MouseDelta.y * 0.01f;
        }
        
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {   // mouse event rotating (right click)
            phi -= io.MouseDelta.y * 0.01f;
            theta -= io.MouseDelta.x * 0.01f;   
        }
        else if (io.MouseWheel != 0)
        {
            scale += 0.02 * io.MouseWheel;
            if (scale > 2.0f) scale = 2.0f;
            else if (scale < 0.2f) scale = 0.2f;
        }     
    }
    // device control event end ////////////////////
    
    // update constant buffer
    /* 
    m_cBuffer.world = XMMatrixTranspose(
        XMMatrixScalingFromVector(XMVectorReplicate(scale)) * 
        XMMatrixRotationX(phi) * XMMatrixRotationY(theta) *
        XMMatrixTranslation(tx, ty, 0.0f));
    m_cBuffer.proj = XMMatrixTranspose
        (XMMatrixPerspectiveFovLH(fov, AspectRatio(), 1.0f, 1000.0f));
    */

    // update constant buffer, let cube rotate!
    D3D11_MAPPED_SUBRESOURCE mppedData;
    // HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mppedData));
    // memcpy_s(mppedData.pData, sizeof(m_cBuffer), &m_cBuffer, sizeof(m_cBuffer));
    // m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
}

bool GameApp::InitResourcePrevious()
{
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

    // vertex buffer ////////////////////////////////////////
    // INIT: vertex buffer description
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

    // CREATE: vertex buffer
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

    // constant buffer //////////////////////////////////////
    // INIT: constant buffer description 
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    // cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // CREATE: constant buffer without init data
    /* 
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));

    // appoint constant buffer data 
    m_cBuffer.world = XMMatrixIdentity();
    m_cBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
    m_cBuffer.proj = XMMatrixTranspose(
        XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));

    m_cBuffer.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_cBuffer.useCustomColor = false; 
    */

    // TODO: here   //?

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
        // bind the constant buffer that already update to the VS (and PS!)
    /*  
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf()); 
    */

    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get() , nullptr, 0);

    // setting debug object name 
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Trangle_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Trangle_PS");

    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");
    // D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Cube_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Cube_PS");

    return true;
}
