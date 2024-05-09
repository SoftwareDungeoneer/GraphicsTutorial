struct Vertex
{
    float3 pos2DwithDepth : POSITION;
    float4 color : COLOR;
};

struct VSOut
{
    float4 pos : SV_Position;
    float4 col : COLOR;
};

VSOut main(Vertex vsIn)
{
    VSOut vsOut =
    {
        float4(vsIn.pos2DwithDepth, 1.f),
        vsIn.color
    };
    
    return vsOut;
}
