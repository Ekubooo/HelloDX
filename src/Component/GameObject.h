#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Effects.h"
#include <Geometry.h>
#include <Transform.h>

class GameObject
{
public:
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    GameObject();

    // get object transform
    Transform& GetTransform();
    const Transform& GetTransform() const;

    template<class VertexType, class IndexType>
    void SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData);

    void SetTexture(ID3D11ShaderResourceView* texture);

    void SetMaterial(const Material& material);

    void Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect);

    // if buffer reset, debug name reset as well
    void SetDebugObjectName(const std::string& name);
private:
    Transform m_Transform;								// object transform data
    Material m_Material;								// object material 
    ComPtr<ID3D11ShaderResourceView> m_pTexture;		// object texture
    ComPtr<ID3D11Buffer> m_pVertexBuffer;				// vertex buffer 
    ComPtr<ID3D11Buffer> m_pIndexBuffer;				// index buffer
    UINT m_VertexStride;								// vertex byte size
    UINT m_IndexCount;								    // index counter
};

template<class VertexType, class IndexType>
inline void GameObject::SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType, IndexType>& meshData)
{
    // release previous resource
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();

    // check device
    if (device == nullptr)
        return;

    // INIT: vertex buffer description
    m_VertexStride = sizeof(VertexType);
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // CREATE: vertex buffer 
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = meshData.vertexVec.data();
    device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf());

    // INIT: index buffer description
    m_IndexCount = (UINT)meshData.indexVec.size();
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // CREATE: index buffer 
    InitData.pSysMem = meshData.indexVec.data();
    device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf());

}


#endif
