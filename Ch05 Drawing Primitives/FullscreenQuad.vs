struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

static const VSOut QuadVerts[4] =
{
    { float4(1, 0, 0, 1), float2(0, 0) }, // Upper left
    { float4(1, 1, 0, 1), float2(1, 0) }, // Upper right
    { float4(-1, 0, 0, 1), float2(0, 1) }, // Lower left
    { float4(-1, 1, 0, 1), float2(1, 1) }, // Lower right
};

VSOut main(uint vertexId : SV_VertexID)
{
    return QuadVerts[vertexId];
}
