#include "Light.hlsli"

// pixal shader
float4 PS(VertexOut pIn) : SV_Target
{
    // normalized normal vec
    pIn.normalW = normalize(pIn.normalW);

    // direction of vertex to eye
    float3 toEyeW = normalize(g_EyePosW - pIn.posW);

    // init value equals 0 
    float4 ambient, diffuse, spec;
    float4 A, D, S;
    ambient = diffuse = spec = A = D = S = float4(0.0f, 0.0f, 0.0f, 0.0f);

    ComputeDirectionalLight(g_Material, g_DirLight, pIn.normalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputePointLight(g_Material, g_PointLight, pIn.posW, pIn.normalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputeSpotLight(g_Material, g_SpotLight, pIn.posW, pIn.normalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    float4 litColor = pIn.color * (ambient + diffuse) + spec;
    
    litColor.a = g_Material.diffuse.a * pIn.color.a;
    
    return litColor;
}