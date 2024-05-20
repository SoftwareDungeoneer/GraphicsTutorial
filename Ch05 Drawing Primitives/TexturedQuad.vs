cbuffer viewport : register(b0)
{
	float2 vpTopLeft;
	float2 vpDimensions;
	float2 vpDepth;
}

struct VSIn
{
	float2 pos : POSITION;
	float2 tex : TEXCOORD;
};

struct VSOut
{
	float4 position : SV_Position;
	float2 tex : TEXCOORD;
};

VSOut main(VSIn vsIn)
{
	VSOut vsOut =
	{
		float4(
			((vsIn.pos.x * 2) / vpDimensions.x) - 1,
			((vsIn.pos.y * 2) / vpDimensions.y) - 1,
			0.f,
			1.f
		),
		vsIn.tex
	};
	
	return vsOut;
}
