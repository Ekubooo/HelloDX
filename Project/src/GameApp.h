#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <CameraController.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
#include <Collision.h>
#include <ModelManager.h>
#include <TextureManager.h>

class GameApp : public D3DApp
{

public:
    enum class SphereMode { None, Reflection, Refraction };
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

private:
    bool InitResource();
    void DrawScene(bool drawCenterSphere, const Camera& camera, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);

    
private:

    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    BasicEffect m_BasicEffect;
    SkyboxEffect m_SkyboxEffect;

    std::unique_ptr<Depth2D> m_pDepthTexture;                   // depth buffer
    std::unique_ptr<TextureCube> m_pDynamicTextureCube;         // dynamic texture
    std::unique_ptr<Depth2D> m_pDynamicCubeDepthTexture;        // dynamic skybox depth buffer
    std::unique_ptr<Texture2D> m_pDebugDynamicCubeTexture;      // debug 

    GameObject m_Spheres[5];
    GameObject m_CenterSphere;
    GameObject m_Ground;
    GameObject m_Cylinders[5];
    GameObject m_Skybox;
    GameObject m_DebugSkybox;

    std::shared_ptr<FirstPersonCamera> m_pCamera;
    std::shared_ptr<FirstPersonCamera> m_pCubeCamera;
    std::shared_ptr<FirstPersonCamera> m_pDebugCamera;
    FirstPersonCameraController m_CameraController;

    ImVec2 m_DebugTextureXY;
    ImVec2 m_DebugTextureWH;

    SphereMode m_SphereMode = SphereMode::Reflection;
    float m_SphereRad = 0.0f;
    float m_Eta = 1.0f / 1.51f;

};


#endif