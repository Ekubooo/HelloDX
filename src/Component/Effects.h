#ifndef EFFECTS_H
#define EFFECTS_H

#include <memory>
#include <LightHelper.h>
#include <RenderStates.h>

class IEffect
{
public:
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    IEffect() = default;
    virtual ~IEffect() = default;
    // ban copy constructor, allow move
    IEffect(const IEffect&) = delete;
    IEffect& operator=(const IEffect&) = delete;
    IEffect(IEffect&&) = default;
    IEffect& operator=(IEffect&&) = default;

    // UPDATE and BIND const buffer
    virtual void Apply(ID3D11DeviceContext * deviceContext) = 0;
};


class BasicEffect : public IEffect
{
public:

    BasicEffect();
    virtual ~BasicEffect() override;

    BasicEffect(BasicEffect&& moveFrom) noexcept;
    BasicEffect& operator=(BasicEffect&& moveFrom) noexcept;

    // singleton get
    static BasicEffect& Get();

    bool InitAll(ID3D11Device * device);

    //
    // Rander state changing 
    //

    // default 
    void SetRenderDefault(ID3D11DeviceContext * deviceContext);
    // Alpha blend
    void SetRenderAlphaBlend(ID3D11DeviceContext * deviceContext);
    // single blend
    void SetRenderNoDoubleBlend(ID3D11DeviceContext * deviceContext, UINT stencilRef);
    // Stencil write only
    void SetWriteStencilOnly(ID3D11DeviceContext * deviceContext, UINT stencilRef);
    // Default with stencil
    void SetRenderDefaultWithStencil(ID3D11DeviceContext * deviceContext, UINT stencilRef);
    // Alpha Blend with stencil
    void SetRenderAlphaBlendWithStencil(ID3D11DeviceContext * deviceContext, UINT stencilRef);
    // 2D default
    void Set2DRenderDefault(ID3D11DeviceContext * deviceContext);
    // 2D Alpha blend
    void Set2DRenderAlphaBlend(ID3D11DeviceContext * deviceContext);

    //
    // matrix setting 
    //

    void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W);
    void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V);
    void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P);

    void XM_CALLCONV SetReflectionMatrix(DirectX::FXMMATRIX R);
    void XM_CALLCONV SetShadowMatrix(DirectX::FXMMATRIX S);
    void XM_CALLCONV SetRefShadowMatrix(DirectX::FXMMATRIX RefS);
    
    //
    // Light, Material and Texture setting 
    //

    // Maximum light 
    static const int maxLights = 5;

    void SetDirLight(size_t pos, const DirectionalLight& dirLight);
    void SetPointLight(size_t pos, const PointLight& pointLight);
    void SetSpotLight(size_t pos, const SpotLight& spotLight);

    void SetMaterial(const Material& material);

    void SetTexture(ID3D11ShaderResourceView * texture);

    void SetEyePos(const DirectX::XMFLOAT3& eyePos);



    //
    // state trigger
    //

    void SetReflectionState(bool isOn);
    void SetShadowState(bool isOn);
    

    // apply(update) constant buffer and texture
    void Apply(ID3D11DeviceContext * deviceContext) override;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};










#endif