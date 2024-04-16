cbuffer input
{
    float4 fillColor;
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
    Output output =
    {
        float4(
			((pos.x * 2) / vpWidth) - 1,
			-((pos.y * 2) / vpHeight - 1),
			1.f,
			1.f
		),
		fillColor
    };
	
    return output;
}
