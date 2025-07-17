#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <cstring>
#include <DirectXMath.h>


// direction light
struct DirectionalLight
{
    DirectionalLight() = default;

    DirectionalLight(const DirectionalLight&) = default;
    DirectionalLight& operator=(const DirectionalLight&) = default;

    DirectionalLight(DirectionalLight&&) = default;
    DirectionalLight& operator=(DirectionalLight&&) = default;

    DirectionalLight(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular,
        const DirectX::XMFLOAT3& _direction) :
        ambient(_ambient), diffuse(_diffuse), specular(_specular), direction(_direction), pad() {}

    DirectX::XMFLOAT4 ambient;
    DirectX::XMFLOAT4 diffuse;
    DirectX::XMFLOAT4 specular;
    DirectX::XMFLOAT3 direction;
    float pad;      // filled with float at last, convenience for HLSL array setting.
    // so struct size meet the demand of several times of 16
};

// point light
struct PointLight
{
    PointLight() = default;

    PointLight(const PointLight&) = default;
    PointLight& operator=(const PointLight&) = default;

    PointLight(PointLight&&) = default;
    PointLight& operator=(PointLight&&) = default;

    PointLight(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular,
        const DirectX::XMFLOAT3& _position, float _range, const DirectX::XMFLOAT3& _att) :
        ambient(_ambient), diffuse(_diffuse), specular(_specular), position(_position), range(_range), att(_att), pad() {}

    DirectX::XMFLOAT4 ambient;
    DirectX::XMFLOAT4 diffuse;
    DirectX::XMFLOAT4 specular;

    // packed into 4D vector: (position, range)
    DirectX::XMFLOAT3 position;
    float range;

    // packed into 4D vector: (A0, A1, A2, pad)
    DirectX::XMFLOAT3 att;
    float pad;      // filled with float at last, convenience for HLSL array setting.
    // so struct size meet the demand of several times of 16
};

// spot light part
struct SpotLight
{
    SpotLight() = default;

    SpotLight(const SpotLight&) = default;
    SpotLight& operator=(const SpotLight&) = default;

    SpotLight(SpotLight&&) = default;
    SpotLight& operator=(SpotLight&&) = default;

    SpotLight(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular,
        const DirectX::XMFLOAT3& _position, float _range, const DirectX::XMFLOAT3& _direction,
        float _spot, const DirectX::XMFLOAT3& _att) :
        ambient(_ambient), diffuse(_diffuse), specular(_specular), 
        position(_position), range(_range), direction(_direction), spot(_spot), att(_att), pad() {}

    DirectX::XMFLOAT4 ambient;
    DirectX::XMFLOAT4 diffuse;
    DirectX::XMFLOAT4 specular;

    // packed into 4D vector: (position, range)
    DirectX::XMFLOAT3 position;
    float range;

    // packed into 4D vector: (direction, spot)
    DirectX::XMFLOAT3 direction;
    float spot;

    // packed into 4D vector: (att, pad)
    DirectX::XMFLOAT3 att;
    float pad;      // filled with float at last, convenience for HLSL array setting.
    // so struct size meet the demand of several times of 16
};

// obj surface material
struct Material
{
    Material() = default;

    Material(const Material&) = default;
    Material& operator=(const Material&) = default;

    Material(Material&&) = default;
    Material& operator=(Material&&) = default;

    Material(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular,
        const DirectX::XMFLOAT4& _reflect) :
        ambient(_ambient), diffuse(_diffuse), specular(_specular), reflect(_reflect) {}

    DirectX::XMFLOAT4 ambient;
    DirectX::XMFLOAT4 diffuse;
    DirectX::XMFLOAT4 specular; // w = specular power
    DirectX::XMFLOAT4 reflect;
};

#endif
