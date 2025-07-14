//***************************************************************************************
// Transform.h by X_Jun(MKXJun) (C) 2018-2022 All Rights Reserved.
// Licensed under the MIT License.
//
// 描述对象缩放、旋转(欧拉角为基础)、平移
// Provide 1st person(free view) and 3rd person cameras.
//***************************************************************************************

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <DirectXMath.h>

class Transform
{
public:
    Transform() = default;
    Transform(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& position);
    ~Transform() = default;

    Transform(const Transform&) = default;
    Transform& operator=(const Transform&) = default;

    Transform(Transform&&) = default;
    Transform& operator=(Transform&&) = default;

    // get object scale 
    DirectX::XMFLOAT3 GetScale() const;
    // get object scale 
    DirectX::XMVECTOR GetScaleXM() const;

    // get Euler angle (circular measure)
    // object rotate by order Z-X-Y
    DirectX::XMFLOAT3 GetRotation() const;
    // get Euler angle (circular measure)
    // object rotate by order Z-X-Y
    DirectX::XMVECTOR GetRotationXM() const;

    // get object position
    DirectX::XMFLOAT3 GetPosition() const;
    // get object position
    DirectX::XMVECTOR GetPositionXM() const;

    // get right axis
    DirectX::XMFLOAT3 GetRightAxis() const;
    // get right axis
    DirectX::XMVECTOR GetRightAxisXM() const;

    // get up axis
    DirectX::XMFLOAT3 GetUpAxis() const;
    // get up axis
    DirectX::XMVECTOR GetUpAxisXM() const;

    // get forward axis
    DirectX::XMFLOAT3 GetForwardAxis() const;
    // get forward axis
    DirectX::XMVECTOR GetForwardAxisXM() const;

    // get world transform matrix
    DirectX::XMFLOAT4X4 GetLocalToWorldMatrix() const;
    // get world transform matrix
    DirectX::XMMATRIX GetLocalToWorldMatrixXM() const;

    // get world transform matrix inverse
    DirectX::XMFLOAT4X4 GetWorldToLocalMatrix() const;    
    // get world transform matrix inverse
    DirectX::XMMATRIX GetWorldToLocalMatrixXM() const;

    // set object scale
    void SetScale(const DirectX::XMFLOAT3& scale);    
    // set object scale
    void SetScale(float x, float y, float z);

    // get Euler angle (circular measure)
    // object rotate by order Z-X-Y
    void SetRotation(const DirectX::XMFLOAT3& eulerAnglesInRadian);
    void SetRotation(float x, float y, float z);

    // set object position
    void SetPosition(const DirectX::XMFLOAT3& position);
    // set object position
    void SetPosition(float x, float y, float z);
    
    // appoint object rotate by euler angle
    void Rotate(const DirectX::XMFLOAT3& eulerAnglesInRadian);
    // appoint rotate around the axis with the ORIGIN as the center
    void RotateAxis(const DirectX::XMFLOAT3& axis, float radian);
    // appoint rotate around the axis with the POINT as the center
    void RotateAround(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT3& axis, float radian);

    // translate along a direction
    void Translate(const DirectX::XMFLOAT3& direction, float magnitude);

    // observe a point
    void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });
    // observe a direction
    void LookTo(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });

    // obtain the rotation Euler angle from the rotation matrix
    static DirectX::XMFLOAT3 GetEulerAnglesFromRotationMatrix(const DirectX::XMFLOAT4X4& rotationMatrix);

private:
    DirectX::XMFLOAT3 m_Scale = { 1.0f, 1.0f, 1.0f };   // scale
    DirectX::XMFLOAT3 m_Rotation = {};                  // euler angle (circular measure)
    DirectX::XMFLOAT3 m_Position = {};                  // Position
};

#endif


