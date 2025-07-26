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
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

private:
    bool InitResource();
    
private:

    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    BasicEffect m_BasicEffect;
    SkyboxEffect m_SkyboxEffect;

    std::unique_ptr<Depth2D> m_pDepthTexture;

    GameObject m_Sphere;
    GameObject m_Ground;
    GameObject m_Cylinder;
    GameObject m_Skybox;

    std::shared_ptr<FirstPersonCamera> m_pCamera;
    FirstPersonCameraController m_CameraController;
};


#endif