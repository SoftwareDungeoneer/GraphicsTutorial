cbuffer instance : register(b0)
{
    float2 position;
    float4 instanceColor;
}

cbuffer viewport : register(b1)
{
    float vpWidth;
    float vpHeight;
};

struct Output
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

Output main(float4 pos : POSITION)
{
    pos.xy += position;
    Output output =
    {
        float4(
			((pos.x * 2) / vpWidth) - 1,
			(pos.y * 2) / vpHeight - 1,
			1.f,
			1.f
		),
		instanceColor
    };
	
    return output;
}
