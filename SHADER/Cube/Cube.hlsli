// matrix can replace by float4x4. if without declearation
// "row_major" matrix defult column major order
// you can declear matrix by announce it as "row_major"
// the tutorial default use column major order
// but we need to transpose the matrix at the top of cpp code

cbuffer ConstantBuffer : register(b0)
{                   
    matrix g_World; 
    matrix g_View;  
    matrix g_Proj;  
    vector g_Color;
    uint g_UseCustomColor;
}

struct VertexIn
{
    float3 posL : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
};
