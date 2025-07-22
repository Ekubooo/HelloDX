#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <Camera.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
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

    // depth buffer
    std::unique_ptr<Depth2D> m_pDepthTexture;

    // GameObject
    GameObject m_House;
    GameObject m_Ground;

    // camera
    std::shared_ptr<Camera> m_pCamera;

};


#endif