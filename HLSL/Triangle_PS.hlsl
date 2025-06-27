#include "Triangle.hlsli"

// pixel shader
float4 PS(VertexOut pIn) : SV_Target
{
    return pIn.color;
}