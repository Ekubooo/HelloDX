#include "GameApp.h"
#include <d3dUtil.h>
#include <DXTrace.h>

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight),
    m_CameraMode(CameraMode::ThirdPerson), 
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
    // auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
    auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        cam3rd->RotateX(io.MouseDelta.y * 0.01f);
        cam3rd->RotateY(io.MouseDelta.x * 0.01f);
    }
    cam3rd->Approach(-io.MouseWheel * 1.0f);

    if (ImGui::Begin("Blending"))
    {
        ImGui::Text("Third Person Mode");
        ImGui::Text("Hold the right mouse button and drag the view");
    }
    ImGui::End();
    ImGui::Render();

    // update const buffer (pre frame)
    m_CBFrame.eyePos = m_pCamera->GetPositionXM();
    m_CBFrame.view = XMMatrixTranspose(m_pCamera->GetViewXM());
    
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

    // draw object (not Transparent) 
    m_pd3dImmediateContext->RSSetState(nullptr);
    m_pd3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

    for (auto& wall : m_Walls)
       wall.Draw(m_pd3dImmediateContext.Get()); 
    m_Floor.Draw(m_pd3dImmediateContext.Get());

    // draw object (Transparent) 
    m_pd3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
    m_pd3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);

    Transform& wireFrameTransform = m_WireFence.GetTransform();
    wireFrameTransform.SetPosition(2.0f, 0.01f, 0.0f);
    m_WireFence.Draw(m_pd3dImmediateContext.Get());
    wireFrameTransform.SetPosition(-2.0f, 0.01f, 0.0f);
    m_WireFence.Draw(m_pd3dImmediateContext.Get());
        // water
    m_Water.Draw(m_pd3dImmediateContext.Get());

    // imgui will trigger Direct3D Draw.
    // need to bind backup_buffer on RP before here.
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob; 
      
    // 2d shader ///////////////////////////////////
    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_2D_VS.cso", L"HLSL\\Basic\\Basic_2D_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader2D.GetAddressOf()));

    HR(m_pd3dDevice->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout2D.GetAddressOf()));

    HR(CreateShaderFromFile(L"HLSL\\Basic\\Basic_2D_PS.cso", L"HLSL\\Basic\\Basic_2D_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader2D.GetAddressOf()));

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

    return true;
}

bool GameApp::InitResource()
{
    // reset mesh now in setBuffer()

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
    // INIT: GameObject 
    ComPtr<ID3D11ShaderResourceView> texture;

    Material material{};
    material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
    
        // Fence Box
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WireFence.dds", nullptr, texture.GetAddressOf()));
    m_WireFence.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateBox());
    m_WireFence.SetTexture(texture.Get());
    m_WireFence.SetMaterial(material);

        // floor 
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
    m_Floor.SetBuffer(m_pd3dDevice.Get(),
        Geometry::CreatePlane(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
    m_Floor.SetTexture(texture.Get());
    m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);
    m_Floor.SetMaterial(material);
    
        // wall 
    m_Walls.resize(4);
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));

    for (int i = 0; i < 4; ++i)
    {   // create four walls 
        m_Walls[i].SetBuffer(m_pd3dDevice.Get(),
            Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 1.5f)));
        m_Walls[i].SetMaterial(material);
        Transform& wallTransform = m_Walls[i].GetTransform();
        wallTransform.SetRotation(-XM_PIDIV2, XM_PIDIV2 * i, 0.0f);
        wallTransform.SetPosition(i % 2 ? -10.0f * (i - 2) : 0.0f, 3.0f, i % 2 == 0 ? -10.0f * (i - 1) : 0.0f);
        m_Walls[i].SetTexture(texture.Get());
    }

        // water 
    material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
    material.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\water.dds", nullptr, texture.ReleaseAndGetAddressOf()));
    m_Water.SetBuffer(m_pd3dDevice.Get(),
        Geometry::CreatePlane(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(10.0f, 10.0f)));
    m_Water.SetTexture(texture.Get());
    m_Water.SetMaterial(material);

    // constant buffer /////////////////////////////////////////////
        // per frame 
    auto camera = std::make_shared<ThirdPersonCamera>();
    m_pCamera = camera;
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
    camera->SetDistance(8.0f);
    camera->SetDistanceMinMax(2.0f, 14.0f);
    camera->SetRotationX(XM_PIDIV4);

        // per OnResize
    m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
    m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());

    // const value in constant buffer 
        // direction light 
    m_CBRarely._dirLight[0].ambient     = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely._dirLight[0].diffuse     = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_CBRarely._dirLight[0].specular    = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely._dirLight[0].direction   = XMFLOAT3(0.0f, -1.0f, 0.0f);
        // point light
    m_CBRarely._pointLight[0].position  = XMFLOAT3(0.0f, 15.0f, 0.0f);
    m_CBRarely._pointLight[0].ambient   = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_CBRarely._pointLight[0].diffuse   = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    m_CBRarely._pointLight[0].specular  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_CBRarely._pointLight[0].att       = XMFLOAT3(0.0f, 0.1f, 0.0f);
    m_CBRarely._pointLight[0].range     = 25.0f;
    m_CBRarely.numDirLight      = 1;
    m_CBRarely.numPointLight    = 1;
    m_CBRarely.numSpotLight     = 0;

    // update constant buffer /////////////////////////////////////
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBChangeOnResize), &m_CBOnResize, sizeof(CBChangeOnResize));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);

    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBChangeRarely), &m_CBRarely, sizeof(CBChangeRarely));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[3].Get(), 0);

    // Initiative Rasterizer state /////////////////////////////////
    RenderStates::InitAll(m_pd3dDevice.Get());

    // input Assemble //////////////////////////////////////////////
        // Primitive Topology, input layout
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());
        // preorder bind const buffer ("preframe update" needs 2 buffer)
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(2, 1, m_pConstantBuffers[2].GetAddressOf());
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);
        // Render state
    m_pd3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
    
        // const buffer for VS and PS, corresponding to const buffer in HLSL register b0/b1
    m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(3, 1, m_pConstantBuffers[3].GetAddressOf());
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
        // blend state
    m_pd3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);

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
    m_Floor.SetDebugObjectName("Floor");
    m_Walls[0].SetDebugObjectName("Walls[0]");
    m_Walls[1].SetDebugObjectName("Walls[1]");
    m_Walls[2].SetDebugObjectName("Walls[2]");
    m_Walls[3].SetDebugObjectName("Walls[3]");
    m_WireFence.SetDebugObjectName("WireFence");

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
    : m_IndexCount(), m_Material(), m_VertexStride()
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

void GameApp::GameObject::SetMaterial(const Material & material)
{
    m_Material = material;
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
    cbDrawing.material = m_Material;

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

void GameApp::GameObject::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
#else
    UNREFERENCED_PARAMETER(name);
#endif

}
