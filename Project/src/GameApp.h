#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include "Waves.h"
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
    
private:

    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    BasicEffect m_BasicEffect;

    std::mt19937 m_RandEngine;
    std::uniform_int_distribution<uint32_t> m_RowRange;
    std::uniform_int_distribution<uint32_t> m_ColRange;
    std::uniform_real_distribution<float> m_MagnitudeRange;

    GameObject m_Land;
    GameObject m_WireFence;
    CpuWaves m_CpuWaves;
    GpuWaves m_GpuWaves;

    std::unique_ptr<Depth2D> m_pDepthTexture;
    std::unique_ptr<Texture2D> m_pLitTexture;

    std::shared_ptr<ThirdPersonCamera> m_pCamera;

    float m_BaseTime = 0.0f;
    int m_WavesMode = 1;
    bool m_EnabledFog = true;

};


#endif