#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <d3dApp.h>
#include <LightHelper.h>
#include <Geometry.h>
#include <Camera.h>
#include <RenderStates.h>

class GameApp : public D3DApp
{
public:
    struct CBChangeEveryDrawing
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX worldInvTranspose;
        Material material;
    };
    struct CBChangeEveryFrame
    {
        DirectX::XMMATRIX view;
        DirectX::XMVECTOR eyePos;
    };
    struct CBChangeOnResize
    {
        DirectX::XMMATRIX proj;
    };
    struct CBChangeRarely
    {
        DirectionalLight _dirLight[10];
        PointLight _pointLight[10];
        SpotLight _spotLight[10];
        int numDirLight;
        int numPointLight;
        int numSpotLight;
        int pad;
    };
    
    class GameObject
    { 
    public:
        GameObject();

        // get transform
        Transform& GetTransform();
        const Transform& GetTransform() const;

        template<class VertexType, class IndexType>
        void SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType, IndexType>& meshData);
        void SetTexture(ID3D11ShaderResourceView * texture);
        void SetMaterial(const Material& material);
        void Draw(ID3D11DeviceContext * deviceContext);
        void SetDebugObjectName(const std::string& name);

    private:
        Transform m_Transform;
        Material m_Material;
        ComPtr<ID3D11ShaderResourceView> m_pTexture;
        ComPtr<ID3D11Buffer> m_pVertexBuffer; 
        ComPtr<ID3D11Buffer> m_pIndexBuffer;
        UINT m_VertexStride;        // vertex size (Byte)
        UINT m_IndexCount;          // obj's index array size
    
    };
    
    enum class CameraMode{FirstPerson, ThirdPerson, Free};

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

    template<class VertexType>
    bool ResetMesh(const Geometry::MeshData<VertexType> &meshData);

private:
    ComPtr<ID3D11InputLayout>   m_pVertexLayout2D;      // input layout
    ComPtr<ID3D11InputLayout>   m_pVertexLayout3D;
    ComPtr<ID3D11Buffer>        m_pConstantBuffers[4];  // const buffer

    GameObject m_WireFence;
    GameObject m_Floor;
    GameObject m_Water;
    std::vector<GameObject> m_Walls;

    ComPtr<ID3D11VertexShader>  m_pVertexShader2D;      
    ComPtr<ID3D11PixelShader>   m_pPixelShader2D;       
    ComPtr<ID3D11VertexShader>  m_pVertexShader3D;      
    ComPtr<ID3D11PixelShader>   m_pPixelShader3D;    

    CBChangeEveryFrame m_CBFrame;                       // update per frame
    CBChangeOnResize m_CBOnResize;                      // update when windows size changed
    CBChangeRarely m_CBRarely;                          // always const

    std::shared_ptr<Camera> m_pCamera;
    CameraMode m_CameraMode;
    
    ComPtr<ID3D11RasterizerState> m_pRSWireframe;       // RS state: line framework only
    bool m_IsWireframeMode;
};


#endif