cbuffer viewport : register(b0)
{
    float vpWidth;
    float vpHeight;
};

struct Input
{
    float4 pos : POSITION;
    float2 instPos : IPOS;
    float4 instCol : ICOLOR;
};

struct Output
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

Output main(Input In)
{
    In.pos.xy += In.instPos;
    Output output =
    {
        float4(
			((In.pos.x * 2) / vpWidth) - 1,
			((In.pos.y * 2) / vpHeight) - 1,
			1.f,
			1.f
		),
		In.instCol
    };
	
    return output;
}
