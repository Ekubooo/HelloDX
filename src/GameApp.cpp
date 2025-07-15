#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight),
    m_CameraMode(CameraMode::FirstPerson), 
    m_CBFrame(),
    m_CBOnResize(), 
    m_CBRarely()
{   }

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

    // init mouse
    
    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
    
    // camera change
    if (m_pCamera != nullptr)
    {
        m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
        m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
        m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());
        
        D3D11_MAPPED_SUBRESOURCE mappedData;
        HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
        memcpy_s(mappedData.pData, sizeof(CBChangeOnResize), &m_CBOnResize, sizeof(CBChangeOnResize));
        m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);
    }

}

void GameApp::UpdateScene(float dt)
{
    // get subclass
    auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
    auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

    Transform& woodCrateTransform = m_WoodCrate.GetTransform();

    ImGuiIO& io = ImGui::GetIO();
    if (m_CameraMode == CameraMode::FirstPerson || m_CameraMode == CameraMode::Free)
    {
        // first view operation
        float d1 = 0.0f, d2 = 0.0f;
        if (ImGui::IsKeyDown(ImGuiKey_W))
            d1 += dt;
        if (ImGui::IsKeyDown(ImGuiKey_S))
            d1 -= dt;
        if (ImGui::IsKeyDown(ImGuiKey_A))
            d2 -= dt;
        if (ImGui::IsKeyDown(ImGuiKey_D))
            d2 += dt;

        if (m_CameraMode == CameraMode::FirstPerson)
            cam1st->Walk(d1 * 6.0f);
        else
            cam1st->MoveForward(d1 * 6.0f);
        cam1st->Strafe(d2 * 6.0f);

        // lock camera position in the range of [-8.9, 8.9]x[-8.9, 8.9]x[0.0, 8.9]
        // threading up not allow
        XMFLOAT3 adjustedPos;
        XMStoreFloat3(&adjustedPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f), XMVectorReplicate(8.9f)));
        cam1st->SetPosition(adjustedPos);

        // first view move both camera and wood box
        if (m_CameraMode == CameraMode::FirstPerson)
            woodCrateTransform.SetPosition(adjustedPos);

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            cam1st->Pitch(io.MouseDelta.y * 0.01f);
            cam1st->RotateY(io.MouseDelta.x * 0.01f);
        }
    }
    else if (m_CameraMode == CameraMode::ThirdPerson)
    {        
        // third view operation
        cam3rd->SetTarget(woodCrateTransform.GetPosition());

        // rorate around object
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            cam3rd->RotateX(io.MouseDelta.y * 0.01f);
            cam3rd->RotateY(io.MouseDelta.x * 0.01f);
        }
        cam3rd->Approach(-io.MouseWheel * 1.0f);
    }

    // update view matrix 
    XMStoreFloat4(&m_CBFrame.eyePos, m_pCamera->GetPositionXM());
    m_CBFrame.view = XMMatrixTranspose(m_pCamera->GetViewXM());

    if (ImGui::Begin("Camera"))
    {
        ImGui::Text("W/S/A/D in FPS/Free camera");
        ImGui::Text("Hold the right mouse button and drag the view");
        ImGui::Text("The box moves only at First Person mode");

        static int curr_item = 0;
        static const char* modes[] = 
        {
            "First Person",
            "Third Person",
            "Free Camera"
        };

        if (ImGui::Combo("Camera Mode", &curr_item, modes, ARRAYSIZE(modes)))
        {
            if (curr_item == 0 && m_CameraMode != CameraMode::FirstPerson)
            {
                if (!cam1st)
                {
                    cam1st = std::make_shared<FirstPersonCamera>();
                    cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
                    m_pCamera = cam1st;
                }

                cam1st->LookTo(woodCrateTransform.GetPosition(),
                    XMFLOAT3(0.0f, 0.0f, 1.0f),
                    XMFLOAT3(0.0f, 1.0f, 0.0f));

                m_CameraMode = CameraMode::FirstPerson;
            }
            else if (curr_item == 1 && m_CameraMode != CameraMode::ThirdPerson)
            {
                if (!cam3rd)
                {
                    cam3rd = std::make_shared<ThirdPersonCamera>();
                    cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
                    m_pCamera = cam3rd;
                }
                XMFLOAT3 target = woodCrateTransform.GetPosition();
                cam3rd->SetTarget(target);
                cam3rd->SetDistance(8.0f);
                cam3rd->SetDistanceMinMax(3.0f, 20.0f);

                m_CameraMode = CameraMode::ThirdPerson;
            }
            else if (curr_item == 2 && m_CameraMode != CameraMode::Free)
            {
                if (!cam1st)
                {
                    cam1st = std::make_shared<FirstPersonCamera>();
                    cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
                    m_pCamera = cam1st;
                }
                // start within above box
                XMFLOAT3 pos = woodCrateTransform.GetPosition();
                XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
                XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
                pos.y += 3;
                cam1st->LookTo(pos, to, up);

                m_CameraMode = CameraMode::Free;
            }
        }
        auto woodPos = woodCrateTransform.GetPosition();
        ImGui::Text("Box Position\n%.2f %.2f %.2f", woodPos.x, woodPos.y, woodPos.z);
        auto cameraPos = m_pCamera->GetPosition();
        ImGui::Text("Camera Position\n%.2f %.2f %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
    }
    ImGui::End();
    ImGui::Render();

    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBChangeEveryFrame), &m_CBFrame, sizeof(CBChangeEveryFrame));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    m_pd3dImmediateContext->ClearRenderTargetView
        (m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::SkyBlue));
    m_pd3dImmediateContext->ClearDepthStencilView
        (m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_WoodCrate.Draw(m_pd3dImmediateContext.Get());
    m_Floor.Draw(m_pd3dImmediateContext.Get());
    for (auto& wall : m_Walls)
        wall.Draw(m_pd3dImmediateContext.Get());

    // imgui will trigger Direct3D Draw.
    // need to bind backup_buffer on RP before here.
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob; 
    
    // 3d shader ///////////////////////////////////
    // vertex shader
    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_3D_VS.cso", L"HLSL\\Basic\\Basic_3D_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader3D.GetAddressOf()));
    // vertex layout 
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout3D.GetAddressOf()));
    // pixal shader
    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_3D_PS.cso", L"HLSL\\Basic\\Basic_3D_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader3D.GetAddressOf()));
    
    // 2d shader ///////////////////////////////////
    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_2D_VS.cso", L"HLSL\\Basic\\Basic_2D_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader2D.GetAddressOf()));

    HR(m_pd3dDevice->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout2D.GetAddressOf()));

    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_2D_PS.cso", L"HLSL\\Basic\\Basic_2D_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader2D.GetAddressOf()));

    return true;
}

bool GameApp::InitResource()
{
    // reset mesh now in setBuffer

    // constant buffer setting /////////////////////////////////////
    // INIT: constant buffer description
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // CREATE: constant buffer for VS and PS 
    cbd.ByteWidth = sizeof(CBChangeEveryDrawing);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
    cbd.ByteWidth = sizeof(CBChangeEveryFrame);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));
    cbd.ByteWidth = sizeof(CBChangeOnResize);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[2].GetAddressOf()));
    cbd.ByteWidth = sizeof(CBChangeRarely);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[3].GetAddressOf()));
    
    // Texture and sampler /////////////////////////////////////////
    // INIT: texture 
        // wood box
    ComPtr<ID3D11ShaderResourceView> texture;
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
    m_WoodCrate.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateBox());
    m_WoodCrate.SetTexture(texture.Get());

        // floor 
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
    m_Floor.SetBuffer(m_pd3dDevice.Get(),
        Geometry::CreatePlane(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
    m_Floor.SetTexture(texture.Get());
    m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);
        // wall 
    m_Walls.resize(4);
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
    for (int i = 0; i < 4; ++i)
    {   // create four walls 
        m_Walls[i].SetBuffer(m_pd3dDevice.Get(),
            Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 1.5f)));
        Transform& transform = m_Walls[i].GetTransform();
        transform.SetRotation(-XM_PIDIV2, XM_PIDIV2 * i, 0.0f);
        transform.SetPosition(i % 2 ? -10.0f * (i - 2) : 0.0f, 3.0f, i % 2 == 0 ? -10.0f * (i - 1) : 0.0f);
        m_Walls[i].SetTexture(texture.Get());
    }

    // INIT: texture sampler state
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

    // constant buffer /////////////////////////////////////////////
    // per frame 
    m_CameraMode = CameraMode::FirstPerson;
    auto camera = std::make_shared<FirstPersonCamera>();
    m_pCamera = camera;
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->LookAt(XMFLOAT3(), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

    // per resize
    m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
    m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());

    // const value in constant buffer 
        // direction light 
    m_CBRarely._dirLight[0].ambient     = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely._dirLight[0].diffuse     = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_CBRarely._dirLight[0].specular    = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely._dirLight[0].direction   = XMFLOAT3(0.0f, -1.0f, 0.0f);
        // point light
    m_CBRarely._pointLight[0].position  = XMFLOAT3(0.0f, 10.0f, 0.0f);
    m_CBRarely._pointLight[0].ambient   = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely._pointLight[0].diffuse   = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_CBRarely._pointLight[0].specular  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely._pointLight[0].att       = XMFLOAT3(0.0f, 0.1f, 0.0f);
    m_CBRarely._pointLight[0].range     = 25.0f;
    m_CBRarely.numDirLight      = 1;
    m_CBRarely.numPointLight    = 1;
    m_CBRarely.numSpotLight     = 0;

        // init material 
    m_CBRarely._material.ambient        = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely._material.diffuse        = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    m_CBRarely._material.specular       = XMFLOAT4(0.1f, 0.1f, 0.1f, 50.0f);

    // update constant buffer /////////////////////////////////////
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBChangeOnResize), &m_CBOnResize, sizeof(CBChangeOnResize));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);

    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBChangeRarely), &m_CBRarely, sizeof(CBChangeRarely));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[3].Get(), 0);

    // Initiative Rasterizer state /////////////////////////////////
    /* 
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthClipEnable = true;
        // line frame mode enable, normal = nullptr
    HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));
    */

    // input Assemble //////////////////////////////////////////////
        // Primitive Topology, input layout
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());

        // bind vertex shader to RP
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);
    
        // const buffer for VS and PS, corresponding to const buffer in HLSL register b0/b1
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(2, 1, m_pConstantBuffers[2].GetAddressOf());

    m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(3, 1, m_pConstantBuffers[3].GetAddressOf());
        // bind pixal shader to RP
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
        // texure loader and sampler
    m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());


    // debug test setting //////////////////////////////////////////
    D3D11SetDebugObjectName(m_pVertexLayout2D.Get(), "VertexPosTexLayout");
    D3D11SetDebugObjectName(m_pVertexLayout3D.Get(), "VertexPosNormalTexLayout");
    D3D11SetDebugObjectName(m_pConstantBuffers[0].Get(), "CBDrawing");
    D3D11SetDebugObjectName(m_pConstantBuffers[1].Get(), "CBFrame");
    D3D11SetDebugObjectName(m_pConstantBuffers[2].Get(), "CBOnResize");
    D3D11SetDebugObjectName(m_pConstantBuffers[3].Get(), "CBRarely");
    D3D11SetDebugObjectName(m_pVertexShader2D.Get(), "Basic_2D_VS");
    D3D11SetDebugObjectName(m_pVertexShader3D.Get(), "Basic_3D_VS");
    D3D11SetDebugObjectName(m_pPixelShader2D.Get(), "Basic_2D_PS");
    D3D11SetDebugObjectName(m_pPixelShader3D.Get(), "Basic_3D_PS");
    D3D11SetDebugObjectName(m_pSamplerState.Get(), "SSLinearWrap");
    m_Floor.SetDebugObjectName("Floor");
    m_WoodCrate.SetDebugObjectName("WoodCrate");
    m_Walls[0].SetDebugObjectName("Walls[0]");
    m_Walls[1].SetDebugObjectName("Walls[1]");
    m_Walls[2].SetDebugObjectName("Walls[2]");
    m_Walls[3].SetDebugObjectName("Walls[3]");

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

// GameObject ///////////////////////////////////////

GameApp::GameObject::GameObject() 
    : m_IndexCount(), m_VertexStride()
{   }

Transform &GameApp::GameObject::GetTransform()
{
    return m_Transform;
}

const Transform &GameApp::GameObject::GetTransform() const
{
    return m_Transform;
}

template <class VertexType, class IndexType>
void GameApp::GameObject::SetBuffer
    (ID3D11Device *device, const Geometry::MeshData<VertexType, IndexType> &meshData)
{
    // reset mesh define here
    // release old resource
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();

    // Vertex Buffer //////////////////////////////////////////////////
    // INIT: vertex buffer description
    m_VertexStride = sizeof(VertexType);
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    // CREATE: vertex buffer
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = meshData.vertexVec.data();
    HR(device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    // index Buffer ///////////////////////////////////////////////////
    // Init: index buffer description 
    m_IndexCount = (UINT)meshData.indexVec.size();
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    // Create: index Buffer
    InitData.pSysMem = meshData.indexVec.data();
    HR(device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));

}

void GameApp::GameObject::SetTexture(ID3D11ShaderResourceView *texture)
{
    m_pTexture = texture;
}

void GameApp::GameObject::Draw(ID3D11DeviceContext *deviceContext)
{
    // set vertex and index buffer
    UINT strides = m_VertexStride;
    UINT offsets = 0;

    deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
    deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // get and modify const buffer that already bind on RP
    ComPtr<ID3D11Buffer> cBuffer = nullptr;
    deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
    CBChangeEveryDrawing cbDrawing;

    // matrix inverse  
    XMMATRIX W = m_Transform.GetLocalToWorldMatrixXM();
    cbDrawing.world = XMMatrixTranspose(W);
    cbDrawing.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

    // update const buffer
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBChangeEveryDrawing), &cbDrawing, sizeof(CBChangeEveryDrawing));
    deviceContext->Unmap(cBuffer.Get(), 0);

    // set Texture 
    deviceContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
    // can start drawing
    deviceContext->DrawIndexed(m_IndexCount, 0, 0);

}

void GameApp::GameObject::SetDebugObjectName(const std::string &name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
#else
    UNREFERENCED_PARAMETER(name);
#endif

}
