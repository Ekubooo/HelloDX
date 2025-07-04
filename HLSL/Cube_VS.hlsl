#include "Cube.hlsli"

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    // MVP
    vOut.posH = mul(float4(vIn.posL, 1.0f), g_World);  
    vOut.posH = mul(vOut.posH, g_View); 
    vOut.posH = mul(vOut.posH, g_Proj); 
    vOut.color = vIn.color;  
    return vOut;
}



/* 

** use mul() instead of "*"
** "*" operater required two matrix has same row and column
** Cij = Aij * Bij
** Alpha defult equals 1.0 

*/
