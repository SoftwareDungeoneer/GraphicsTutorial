
cbuffer viewport : register(b0)
{
    float4 fillColor;
    float viewportWidth;
    float viewportHeight;
}

struct Input
{
    float4 pos : POSITION;
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
			((In.pos.x * 2) / viewportWidth) - 1,
			-(((In.pos.y * 2) / viewportHeight) - 1),
			1.f,
			1.f
		),
        fillColor
    };
	
    return output;
}
