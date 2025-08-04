#ifndef EFFECTS_H
#define EFFECTS_H

#include <IEffect.h>
#include <Material.h>
#include <MeshData.h>

class ForwardEffect : public IEffect, public IEffectTransform,
    public IEffectMaterial, public IEffectMeshData
{
public:
    ForwardEffect();
    virtual ~ForwardEffect() override;

    ForwardEffect(ForwardEffect&& moveFrom) noexcept;
    ForwardEffect& operator=(ForwardEffect&& moveFrom) noexcept;

    // singleton get
    static ForwardEffect& Get();

    bool InitAll(ID3D11Device* device);

    void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
    void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
    void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

    void SetMaterial(const Material& material) override;

    MeshDataInput GetInputData(const MeshData& meshData) override;

    void SetLightBuffer(ID3D11ShaderResourceView* lightBuffer);

    void SetLightingOnly(bool enable);
    void SetFaceNormals(bool enable);
    void SetVisualizeLightCount(bool enable);

    // defualt pass draw 
    void SetRenderDefault();

    // Pre-Z pass draw 
    void SetRenderPreZPass();

    // constant buffer and texture resource apply 
    void Apply(ID3D11DeviceContext* deviceContext) override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

class SkyboxEffect : public IEffect, public IEffectTransform,
    public IEffectMeshData, public IEffectMaterial
{
public:
    SkyboxEffect();
    virtual ~SkyboxEffect() override;

    SkyboxEffect(SkyboxEffect&& moveFrom) noexcept;
    SkyboxEffect& operator=(SkyboxEffect&& moveFrom) noexcept;

    static SkyboxEffect& Get();

    bool InitAll(ID3D11Device* device);

    void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;

    void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
    void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

    void SetMaterial(const Material& material) override;


    MeshDataInput GetInputData(const MeshData& meshData) override;

    void SetRenderDefault();

    // depth texture 
    void SetDepthTexture(ID3D11ShaderResourceView* depthTexture);
    // Set: scene render texture 
    void SetLitTexture(ID3D11ShaderResourceView* litTexture);

    // MSAA sample level
    void SetMsaaSamples(UINT msaaSamples);

    void Apply(ID3D11DeviceContext* deviceContext) override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

class DeferredEffect : public IEffect, public IEffectTransform,
    public IEffectMaterial, public IEffectMeshData
{
public:

    DeferredEffect();
    virtual ~DeferredEffect() override;

    DeferredEffect(DeferredEffect&& moveFrom) noexcept;
    DeferredEffect& operator=(DeferredEffect&& moveFrom) noexcept;

    static DeferredEffect& Get();

    bool InitAll(ID3D11Device* device);

    void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) override;
    void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) override;
    void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) override;

    void SetMaterial(const Material& material) override;

    MeshDataInput GetInputData(const MeshData& meshData) override;

    void SetMsaaSamples(UINT msaaSamples);

    void SetLightingOnly(bool enable);
    void SetFaceNormals(bool enable);
    void SetVisualizeLightCount(bool enable);
    void SetVisualizeShadingFreq(bool enable);

    void SetCameraNearFar(float nearZ, float farZ);

    // GBuffer write 
    void SetRenderGBuffer();

    // render normal G-Buffer to target Texture
    void DebugNormalGBuffer(ID3D11DeviceContext* deviceContext,
        ID3D11RenderTargetView* rtv,
        ID3D11ShaderResourceView* normalGBuffer,
        D3D11_VIEWPORT viewport);

    // render ZGrad G-Buffer to target Texture
    void DebugPosZGradGBuffer(ID3D11DeviceContext* deviceContext,
        ID3D11RenderTargetView* rtv,
        ID3D11ShaderResourceView* posZGradGBuffer,
        D3D11_VIEWPORT viewport);

    // classic defer RP
    void ComputeLightingDefault(ID3D11DeviceContext* deviceContext,
        ID3D11RenderTargetView* litBufferRTV,
        ID3D11DepthStencilView* depthBufferReadOnlyDSV,
        ID3D11ShaderResourceView* lightBufferSRV,
        ID3D11ShaderResourceView* GBuffers[4],
        D3D11_VIEWPORT viewport);

    void Apply(ID3D11DeviceContext* deviceContext) override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif
