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
	VSOut vsOut = {
		float4(position, 0, 1),
		color
	};
	
	float theta = TAU / numVerts;
	vsOut.pos.xy = (radius * float2(cos(theta), sin(theta))) + position;
	return vsOut;
}
