//***************************************************************************************
// Camera.h by X_Jun(MKXJun) (C) 2018-2022 All Rights Reserved.
// Licensed under the MIT License.
//
// 提供第一人称(自由视角)和第三人称摄像机
// Provide 1st person(free view) and 3rd person cameras.
//***************************************************************************************

#ifndef CAMERA_H
#define CAMERA_H

#include "WinMin.h"
#include <d3d11_1.h>
#include <DirectXMath.h>
#include "Transform.h"


class Camera
{
public:
    Camera() = default;
    virtual ~Camera() = 0;

    //
    // get camera position
    //

    DirectX::XMVECTOR GetPositionXM() const;
    DirectX::XMFLOAT3 GetPosition() const;

    //
    // get camera rotation
    //

    // get euler angle radians that rotating around the X axis
    float GetRotationX() const;
    // get euler angle radians that rotating around the Y axis
    float GetRotationY() const;

    //
    // get camera axis vector
    //

    DirectX::XMVECTOR GetRightAxisXM() const;
    DirectX::XMFLOAT3 GetRightAxis() const;
    DirectX::XMVECTOR GetUpAxisXM() const;
    DirectX::XMFLOAT3 GetUpAxis() const;
    DirectX::XMVECTOR GetLookAxisXM() const;
    DirectX::XMFLOAT3 GetLookAxis() const;

    //
    // get matrix
    //

    DirectX::XMMATRIX GetViewXM() const;
    DirectX::XMMATRIX GetProjXM() const;
    DirectX::XMMATRIX GetViewProjXM() const;

    // get viewport
    D3D11_VIEWPORT GetViewPort() const;


    // set viewing cone
    void SetFrustum(float fovY, float aspect, float nearZ, float farZ);

    // set viewport
    void SetViewPort(const D3D11_VIEWPORT& viewPort);
    void SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

protected:

    // transformation of camera
    Transform m_Transform = {};
    
    // attribute of viewing cone
    float m_NearZ = 0.0f;
    float m_FarZ = 0.0f;
    float m_Aspect = 0.0f;
    float m_FovY = 0.0f;

    // current virwport
    D3D11_VIEWPORT m_ViewPort = {};

};

class FirstPersonCamera : public Camera
{
public:
    FirstPersonCamera() = default;
    ~FirstPersonCamera() override;

    // set camera position
    void SetPosition(float x, float y, float z);
    void SetPosition(const DirectX::XMFLOAT3& pos);
    // set camera orientation 
    void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target,const DirectX::XMFLOAT3& up);
    void LookTo(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& to, const DirectX::XMFLOAT3& up);
    // translation
    void Strafe(float d);
    // walk (Plane movement)
    void Walk(float d);
    // MoveForward
    void MoveForward(float d);
    // observation pitch and yaw 
    // Positive rad value is up
    // negative rad value is down
    void Pitch(float rad);
    // observation right and left  
    // Positive rad value is right    
    // negative rad value is left
    void RotateY(float rad);
};

class ThirdPersonCamera : public Camera
{
public:
    ThirdPersonCamera() = default;
    ~ThirdPersonCamera() override;

    // get location of current selected object 
    DirectX::XMFLOAT3 GetTargetPosition() const;
    // get distance to object 
    float GetDistance() const;
    // vertical rotate around object 
    // note: euler angle of x axis in range of [0, pi/3]
    void RotateX(float rad);
    // horizontal rotate around object
    void RotateY(float rad);
    // pull colser object
    void Approach(float dist);
    // init euler angle value around X axis
    // note: euler angle of x axis in range of [0, pi/3]
    void SetRotationX(float rad);
    // init euler angle value around Y axis
    void SetRotationY(float rad);
    // set and bind position of object that selected but untracked并绑定待跟踪物体的位置
    void SetTarget(const DirectX::XMFLOAT3& target);
    // init distance
    void SetDistance(float dist);
    // set Maximum and Minimum distance 
    void SetDistanceMinMax(float minDist, float maxDist);

private:
    DirectX::XMFLOAT3 m_Target = {};
    float m_Distance = 0.0f;
    // maximum and minimum allowed distance
    float m_MinDist = 0.0f, m_MaxDist = 0.0f;
};


#endif
