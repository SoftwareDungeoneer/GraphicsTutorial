cbuffer viewport : register(b0)
{
	float2 vpTopLeft;
	float2 vpDimensions;
	float2 vpDepth;
}

cbuffer instanceData : register(b1)
{
	float2 position;
	float radius;
	uint numVerts;
	float4 color;
};

static const float PI  = 3.14159265f;
static const float TAU = 6.2831853f;

struct VSOut
{
	float4 pos : SV_Position;
	float4 color : COLOR;
};

VSOut main(uint vertid : SV_VertexID)
{
	
    float theta = (TAU / numVerts) * (vertid >> 1);
    float r = radius * (~vertid & 1);
    float2 ssxy = r * float2(cos(theta + PI/2), sin(theta + PI/2)) + position;
    ssxy = (2 * ssxy / vpDimensions) - 1;
    VSOut vsOut =
    {
        float4(ssxy, 0, 1),
		color	
    };
	
	return vsOut;
}
