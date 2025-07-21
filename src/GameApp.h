#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <d3dApp.h>
#include <Camera.h>
#include "GameObject.h"

class GameApp : public D3DApp
{
public:
    enum class CameraMode{FirstPerson, ThirdPerson, Free};

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
    GameObject m_WoodCrate;		
    GameObject m_Floor;
    GameObject m_Mirror;
    std::vector<GameObject> m_Walls;

    Material m_ShadowMat;
    Material m_WoodCrateMat;

    BasicEffect m_BasicEffect;

    std::shared_ptr<Camera> m_pCamera;
    CameraMode m_CameraMode;

};


#endif