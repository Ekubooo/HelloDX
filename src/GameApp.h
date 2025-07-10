#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "LightHelper.h"
#include "Geometry.h"

class GameApp : public D3DApp
{
public:
    struct VSConstantBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX worldInvTranspose;
    };

    struct PSConstantBuffer
    {
        DirectionalLight dirLight;
        PointLight pointLight;
        SpotLight spotLight;
        Material material;  
        DirectX::XMFLOAT4 eyePos;
    };

public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

private:
    bool InitEffect();
    bool InitResource();
    bool ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData);

private:
    ComPtr<ID3D11InputLayout>   m_pVertexLayout;        // input layout
    ComPtr<ID3D11Buffer>        m_pVertexBuffer;        // vertex buffer     
    ComPtr<ID3D11Buffer>        m_pIndexBuffer;         // index buffer
    ComPtr<ID3D11Buffer>        m_pConstantBuffers[2];   // index buffer
    UINT m_IndexCount;                                  // obj's index array size

    ComPtr<ID3D11VertexShader>  m_pVertexShader;        // vertex shader
    ComPtr<ID3D11PixelShader>   m_pPixelShader;         // fragment shader
    
    VSConstantBuffer m_VSConstantBuffer;                // GPU constant buffer for vs
    PSConstantBuffer m_PSConstantBuffer;                // GPU constant buffer for ps
    
    DirectionalLight m_DirLight;    // defualt direction light
    PointLight m_PointLight;        // defualt point light 
    SpotLight m_SpotLight;          // defualt spot light

    ComPtr<ID3D11RasterizerState> m_pRSWireframe;       // RS state: line framework only
    bool m_IsWireframeMode;
};


#endif