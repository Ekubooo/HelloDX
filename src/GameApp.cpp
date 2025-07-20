#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight),
    m_CameraMode(CameraMode::ThirdPerson), 
    m_ShadowMat(),
    m_WoodCrate()
{   }

GameApp::~GameApp() 
{

}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    RenderStates::InitAll(m_pd3dDevice.Get()); 

    if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!InitResource())
        return false;

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
        m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
    }

}

void GameApp::UpdateScene(float dt)
{
    // get subclass
    auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
    auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);\

    Transform& woodCrateTransform = m_WoodCrate.GetTransform();

    ImGuiIO& io = ImGui::GetIO();

    if (m_CameraMode == CameraMode::FirstPerson || m_CameraMode == CameraMode::Free)
    {
        // first and free perspective
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

        // position lock in the range of [-8.9f, 8.9f]
        // ban threading
        XMFLOAT3 adjustedPos;
        XMStoreFloat3(&adjustedPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f), XMVectorReplicate(8.9f)));
        cam1st->SetPosition(adjustedPos);

        // movement allowed only in first perspective
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
        // third perspective 
        cam3rd->SetTarget(woodCrateTransform.GetPosition());

        // rotate around object
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            cam3rd->RotateX(io.MouseDelta.y * 0.01f);
            cam3rd->RotateY(io.MouseDelta.x * 0.01f);
        }
        cam3rd->Approach(-io.MouseWheel * 1.0f);
    }

    // set dirty state, waiting for update ?
    m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
    m_BasicEffect.SetEyePos(m_pCamera->GetPosition());

    // imgui componment
    if (ImGui::Begin("Living without FX11"))
    {
        ImGui::Text("W/S/A/D in FPS/Free camera");
        ImGui::Text("Hold the right mouse button and drag the view");
        ImGui::Text("The box moves only at First Person mode");

        static int curr_item = 1;

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
                cam3rd->SetDistance(5.0f);
                cam3rd->SetDistanceMinMax(2.0f, 14.0f);

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
                // 从箱子上方开始
                XMFLOAT3 pos = woodCrateTransform.GetPosition();
                XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
                XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
                pos.y += 3;
                cam1st->LookTo(pos, to, up);

                m_CameraMode = CameraMode::Free;
            }
        }
    }
    ImGui::End();
    ImGui::Render();

}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    m_pd3dImmediateContext->ClearRenderTargetView
        (m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::SkyBlue));
    m_pd3dImmediateContext->ClearDepthStencilView
        (m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // draw operation /////////////////////////////////////////////////////////
    // 01 mirror Stencil
    m_BasicEffect.SetWriteStencilOnly(m_pd3dImmediateContext.Get(), 1);
    m_Mirror.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    // 02 mirror non-transparent object
        // reflection draw 
    m_BasicEffect.SetReflectionState(true);
    m_BasicEffect.SetRenderDefaultWithStencil(m_pd3dImmediateContext.Get(), 1);

    m_Walls[2].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_Walls[3].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_Walls[4].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    // 03 shadow of mirror non-transparent object
    m_WoodCrate.SetMaterial(m_ShadowMat);
        // reflection on, shadow on 
    m_BasicEffect.SetShadowState(true);			
    m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 1);

    m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        // recover state
    m_BasicEffect.SetShadowState(false);
    m_WoodCrate.SetMaterial(m_WoodCrateMat);

    // 04 transparent mirror 
        // reflection draw off
    m_BasicEffect.SetReflectionState(false);
    m_BasicEffect.SetRenderAlphaBlendWithStencil(m_pd3dImmediateContext.Get(), 1);

    m_Mirror.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    // 05 draw non-transparent object
    m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get());

    for (auto& wall : m_Walls)
        wall.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    // 06 shadow of non-transparent object
    m_WoodCrate.SetMaterial(m_ShadowMat);
        // reflection off, shadow on
    m_BasicEffect.SetShadowState(true);
    m_BasicEffect.SetRenderNoDoubleBlend(m_pd3dImmediateContext.Get(), 0);

    m_WoodCrate.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        // shadow off
    m_BasicEffect.SetShadowState(false);		
    m_WoodCrate.SetMaterial(m_WoodCrateMat);

    // end draw operation /////////////////////////////////////////////////////

    // imgui will trigger Direct3D Draw.
    // need to bind backup_buffer on RP before here.
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitResource()
{
    // GameObject Setting //////////////////////////////////////////
    ComPtr<ID3D11ShaderResourceView> texture;

    Material material{};
    material.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);
    
    m_WoodCrateMat = material;
    m_ShadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_ShadowMat.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
    m_ShadowMat.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
        // wood Box
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
    m_WoodCrate.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateBox());
    m_WoodCrate.GetTransform().SetPosition(0.0f, 0.01f, 5.0f);
    m_WoodCrate.SetTexture(texture.Get());
    m_WoodCrate.SetMaterial(material);

        // floor 
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
    m_Floor.SetBuffer(m_pd3dDevice.Get(),
        Geometry::CreatePlane(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
    m_Floor.SetTexture(texture.Get());
    m_Floor.SetMaterial(material);
    m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);
    
        // wall 
    // five walls, Mirror between 0 and 1
    /*    ____    ____
        /| 0 |   | 1 |\
       /4|___|___|___|2\
      /_/_ _ _ _ _ _ _\_\
     | /       3       \ |
     |/_________________\|
    */

    m_Walls.resize(5);
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
    
    for (int i = 0; i < 5; ++i)
    {
        m_Walls[i].SetMaterial(material);
        m_Walls[i].SetTexture(texture.Get());
    }
    m_Walls[0].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(6.0f, 8.0f), XMFLOAT2(1.5f, 2.0f)));
    m_Walls[1].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(6.0f, 8.0f), XMFLOAT2(1.5f, 2.0f)));
    m_Walls[2].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
    m_Walls[3].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
    m_Walls[4].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
    
    m_Walls[0].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
    m_Walls[0].GetTransform().SetPosition(-7.0f, 3.0f, 10.0f);
    m_Walls[1].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
    m_Walls[1].GetTransform().SetPosition(7.0f, 3.0f, 10.0f);
    m_Walls[2].GetTransform().SetRotation(-XM_PIDIV2, XM_PIDIV2, 0.0f);
    m_Walls[2].GetTransform().SetPosition(10.0f, 3.0f, 0.0f);
    m_Walls[3].GetTransform().SetRotation(-XM_PIDIV2, XM_PI, 0.0f);
    m_Walls[3].GetTransform().SetPosition(0.0f, 3.0f, -10.0f);
    m_Walls[4].GetTransform().SetRotation(-XM_PIDIV2, -XM_PIDIV2, 0.0f);
    m_Walls[4].GetTransform().SetPosition(-10.0f, 3.0f, 0.0f);

        // mirror
    material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
    material.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\ice.dds", nullptr, texture.ReleaseAndGetAddressOf()));
    m_Mirror.SetBuffer(m_pd3dDevice.Get(),
        Geometry::CreatePlane(XMFLOAT2(8.0f, 8.0f), XMFLOAT2(1.0f, 1.0f)));
    m_Mirror.GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
    m_Mirror.GetTransform().SetPosition(0.0f, 3.0f, 10.0f);
    m_Mirror.SetTexture(texture.Get());
    m_Mirror.SetMaterial(material);

    // camera //////////////////////////////////////////////////////
    auto camera = std::make_shared<ThirdPersonCamera>();
    m_pCamera = camera;
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->SetDistance(5.0f);
    camera->SetDistanceMinMax(2.0f, 14.0f);
    camera->SetRotationX(XM_PIDIV2);

    m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
    m_BasicEffect.SetEyePos(m_pCamera->GetPosition());

    m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);

    m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());

    // init const value
    m_BasicEffect.SetReflectionMatrix(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));
    // Tricky position to show shadow 
    m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, -10.0f, 1.0f)));
    m_BasicEffect.SetRefShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f)));

    // environment light 
    DirectionalLight dirLight;
    dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
    m_BasicEffect.SetDirLight(0, dirLight);

    // light 
    PointLight pointLight;
    pointLight.position = XMFLOAT3(0.0f, 10.0f, -10.0f);
    pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    pointLight.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
    pointLight.range = 25.0f;
    m_BasicEffect.SetPointLight(0, pointLight);

    // debug object name 
    m_Floor.SetDebugObjectName("Floor");
    m_Mirror.SetDebugObjectName("Mirror");
    m_Walls[0].SetDebugObjectName("Walls[0]");
    m_Walls[1].SetDebugObjectName("Walls[1]");
    m_Walls[2].SetDebugObjectName("Walls[2]");
    m_Walls[3].SetDebugObjectName("Walls[3]");
    m_Walls[4].SetDebugObjectName("Walls[4]");
    m_WoodCrate.SetDebugObjectName("WoodCrate");

    return true;
}
