struct VertexIn
{
    float3 pos : POSITIONT;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float4 color : COLOR;
};
