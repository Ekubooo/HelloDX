#include "GameApp.h"
#include <XUtil.h>
#include <DXTrace.h>

#include <iostream>
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight)
{   }

GameApp::~GameApp() 
{   }

bool GameApp::Init()
{
    std::cout<<"D3Dapp init start"<<std::endl;
    if (!D3DApp::Init())
        return false;
    std::cout<<"D3Dapp init success"<<std::endl;

    m_TextureManager.Init(m_pd3dDevice.Get());
    std::cout<<"texture init success"<<std::endl;

    m_ModelManager.Init(m_pd3dDevice.Get());
    std::cout<<"Model init success"<<std::endl;


    RenderStates::InitAll(m_pd3dDevice.Get()); 
    std::cout<<"RS init success"<<std::endl;


    if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    std::cout<<"Basic Effect init success"<<std::endl;

    if (!InitResource())
        return false;

    std::cout<<"InitResource success"<<std::endl;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
    
    m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
    m_pDepthTexture->SetDebugObjectName("DepthTexture");
    
    // camera change
    if (m_pCamera != nullptr)
    {
        m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
        m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
        m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
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

    if (ImGui::Begin("Meshes"))
    {
        ImGui::Text("Third Person Mode");
        ImGui::Text("Hold the right mouse button and drag the view");
    }
    ImGui::End();
    ImGui::Render();

    m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_BasicEffect.SetEyePos(m_pCamera->GetPosition());

}

void GameApp::DrawScene()
{
    // CREATE Backup Buffer of Render target view
    if (m_FrameCount < m_BackBufferCount)
    {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
        CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc
            (D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        m_pd3dDevice->CreateRenderTargetView
            (pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
    }

    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float skyblue[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
    // m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), reinterpret_cast<const float*>(&Colors::SkyBlue));
    m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), skyblue);
    m_pd3dImmediateContext->ClearDepthStencilView
        (m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ID3D11RenderTargetView* pRTVs[1] = { GetBackBufferRTV() };
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
    D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
    m_pd3dImmediateContext->RSSetViewports(1, &viewport);

    m_BasicEffect.SetRenderDefault();
    m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_House.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));

}

bool GameApp::InitResource()
{
    // GameObject Init /////////////////////////////////////////////
        // ground
    Model* pModel = m_ModelManager.CreateFromFile("..\\Assets\\Model\\ground_19.obj");
    m_Ground.SetModel(pModel);
    pModel->SetDebugObjectName("ground_19");
        // house
    pModel = m_ModelManager.CreateFromFile("..\\Assets\\Model\\house.obj");
    m_House.SetModel(pModel);
    pModel->SetDebugObjectName("house");

        // house bounding box
    XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);
    BoundingBox houseBox = m_House.GetModel()->boundingbox;
    houseBox.Transform(houseBox, S);

        // ground position 
    Transform& houseTransform = m_House.GetTransform();
    houseTransform.SetScale(0.015f, 0.015f, 0.015f);
    houseTransform.SetPosition(0.0f, -(houseBox.Center.y - houseBox.Extents.y + 1.0f), 0.0f);
    
    // camera //////////////////////////////////////////////////////
    auto camera = std::make_shared<ThirdPersonCamera>();
    m_pCamera = camera;
    
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
    camera->SetDistance(15.0f);
    camera->SetDistanceMinMax(6.0f, 100.0f);
    camera->SetRotationX(XM_PIDIV4);
    camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);

    m_BasicEffect.SetWorldMatrix(XMMatrixIdentity());
    m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());
    m_BasicEffect.SetEyePos(camera->GetPosition());

    // init const value ////////////////////////////////////////////
        // environment 
    DirectionalLight dirLight{};
    dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
    m_BasicEffect.SetDirLight(0, dirLight);

        // light 
    PointLight pointLight{};
    pointLight.position = XMFLOAT3(0.0f, 20.0f, 0.0f);
    pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    pointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
    pointLight.range = 30.0f;	
    m_BasicEffect.SetPointLight(0, pointLight);

    return true;
}
