cbuffer viewport : register(b0)
{
	float2 vpTopLeft;
	float2 vpDimensions;
	float2 vpDepth;
}

float4 NDCFromVec2SS(float2 v)
{
    return float4(((2 * v) / vpDimensions) - 1, 0, 1);
}

float4 main(float2 posIn : POSITION) : SV_Position
{
	return NDCFromVec2SS(vsIn.pos);
}
