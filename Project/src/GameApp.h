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

struct PointLight
{
    DirectX::XMFLOAT3 posV;
    float attenuationBegin;
    DirectX::XMFLOAT3 color;
    float attenuationEnd;
};

struct PointLightInitData
{
    float radius;
    float angle;
    float height;
    float animationSpeed;
};



class GameApp : public D3DApp
{
public:
    // light source culling 
    enum class LightCullTechnique 
    {
        CULL_FORWARD_NONE = 0,    // forward rendering no culling
        CULL_FORWARD_PREZ_NONE,   // Forward with prewrite depth
        CULL_DEFERRED_NONE,       // Classic deferred rendering
    };

    const UINT MAX_LIGHTS = 1024;
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth = 1280, int initHeight = 720);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

private:
    bool InitResource();
    void InitLightParams();
    
    DirectX::XMFLOAT3 HueToRGB(float hue);

    void ResizeLights(UINT activeLights);
    void UpdateLights(float dt);

    // after frame resource update 
    void XM_CALLCONV SetupLights(DirectX::XMMATRIX viewMatrix);

    void ResizeBuffers(UINT width, UINT height, UINT msaaSamples);

    void RenderForward(bool doPreZ);
    void RenderGBuffer();
    void RenderSkybox();

private:
    
    // GPU timing
    GpuTimer m_GpuTimer_PreZ;
    GpuTimer m_GpuTimer_Geometry;
    GpuTimer m_GpuTimer_Lighting;
    GpuTimer m_GpuTimer_Skybox;

    // setting
    LightCullTechnique m_LightCullTechnique = LightCullTechnique::CULL_DEFERRED_NONE;
    bool m_AnimateLights = false;
    bool m_LightingOnly = false;
    bool m_FaceNormals = false;
    bool m_VisualizeLightCount = false;
    bool m_VisualizeShadingFreq = false;
    bool m_ClearGBuffers = false;
    float m_LightHeightScale = 0.25f;


    // resource
    TextureManager m_TextureManager;                                // Texture loader 
    ModelManager m_ModelManager;									// Model loader 
    UINT m_MsaaSamples = 1;
    bool m_MsaaSamplesChanged = false;
    std::unique_ptr<Texture2DMS> m_pLitBuffer;                      // sence render buffer 
    std::unique_ptr<Depth2DMS> m_pDepthBuffer;                      // depth buffer 
    ComPtr<ID3D11DepthStencilView> m_pDepthBufferReadOnlyDSV;       // depth Stenci view read only
    std::vector<std::unique_ptr<Texture2DMS>> m_pGBuffers;          // G-Buffers
    std::unique_ptr<Texture2D> m_pDebugNormalGBuffer;               // debug normal Gbuffer 
    std::unique_ptr<Texture2D> m_pDebugPosZGradGBuffer;			    // debug ZGrad Gbuffer 
    std::unique_ptr<Texture2D> m_pDebugAlbedoGBuffer;               // debug Albedo Gbuffer 

    // single pass
    std::vector<ID3D11RenderTargetView*> m_pGBufferRTVs;
    std::vector<ID3D11ShaderResourceView*> m_pGBufferSRVs;

    // 光照
    UINT m_ActiveLights = (MAX_LIGHTS >> 2);
    std::vector<PointLightInitData> m_PointLightInitDatas;
    std::vector<PointLight> m_PointLightParams;
    std::vector<DirectX::XMFLOAT3> m_PointLightPosWorlds;
    std::unique_ptr<StructuredBuffer<PointLight>> m_pLightBuffer;   // point light buffer

    // 模型
    GameObject m_Sponza;											// sence model
    GameObject m_Skybox;											// skybox

    // 特效
    ForwardEffect m_ForwardEffect;				                    // Forward RP
    DeferredEffect m_DeferredEffect;				                // Deferred RP
    SkyboxEffect m_SkyboxEffect;			                        // skybox 

    // 摄像机
    std::shared_ptr<Camera> m_pCamera;								// camera
    FirstPersonCameraController m_FPSCameraController;				// controller

};


#endif