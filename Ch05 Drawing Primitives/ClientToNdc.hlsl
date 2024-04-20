cbuffer viewport : register(b0)
{
    float2 vpTopLeft;
    float2 vpDimensions;
    float2 vpDepth;
}

struct Input
{
    float4 pos : POSITION;
    float4 color : COLOR;
};

struct Output
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

Output main(Input In)
{
    Output output =
    {
        float4(
			((In.pos.x * 2) / vpDimensions.x) - 1,
			((In.pos.y * 2) / vpDimensions.y) - 1,
			1.f,
			1.f
		),
		In.color
    };
	
    return output;
}
