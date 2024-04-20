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
	float angle;
	float4 color;
	uint numVerts;
	uint mode;
};

static const float PI  = 3.14159265f;
static const float TAU = 6.2831853f;

struct VSOut
{
	float4 pos : SV_Position;
	float4 color : COLOR;
};

float2 ComputeNextFilledVertex(uint vertid, float2 pos)
{
	float theta = (TAU / numVerts) * (vertid >> 1) + angle;
	float r = radius * (~vertid & 1);
	float2 ssxy = r * float2(cos(theta + PI / 2), sin(theta + PI / 2)) + pos;
	return ssxy;
}

float2 ComputeNextOutlineVertex(uint vertid, float2 pos)
{
	float th = (TAU / numVerts) * (vertid % numVerts) + angle;
	float ra = radius;
	float2 ssxy = ra * float2(cos(th + PI / 2), sin(th + PI / 2)) + pos;
	return ssxy;
}

VSOut main(uint vertid : SV_VertexID)
{
	float2 filledVert = ComputeNextFilledVertex(vertid, position);
	float2 outlineVert = ComputeNextOutlineVertex(vertid, position);
	float2 posOut = (filledVert * (1 - mode)) + (outlineVert * mode);
	posOut = (2 * posOut / vpDimensions) - 1;
	
	VSOut vsOut =
	{
		float4(posOut, 0, 1),
		color
	};
	
	return vsOut;
}
