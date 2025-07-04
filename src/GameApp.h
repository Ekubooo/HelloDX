#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
class GameApp : public D3DApp
{
public:
    struct VertexPosColor
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
    };  // contrast with struct of vertex shader.

    struct ConstantBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMFLOAT4 color;
        uint32_t useCustomColor;
        uint32_t pads[3];
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

private:
    ComPtr<ID3D11InputLayout>   m_pVertexLayout;    // input layout
    ComPtr<ID3D11Buffer>        m_pVertexBuffer;    // vertex buffer     
    ComPtr<ID3D11Buffer>        m_pIndexBuffer;     // index buffer
    ComPtr<ID3D11Buffer>        m_pConstantBuffer;  // index buffer

    ComPtr<ID3D11VertexShader>  m_pVertexShader;    // vertex shader
    ComPtr<ID3D11PixelShader>   m_pPixelShader;     // fragment shader
    ConstantBuffer              m_cBuffer;          // for GPU constant buffer modify
};


#endif