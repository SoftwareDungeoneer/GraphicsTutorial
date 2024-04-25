cbuffer LineParams
{
    float2 begin;
    float2 end;
    uint falloffFunction;
    float falloffMax;
    float scale;
    float clip;
    float2 rationalPowers;
	float2 reserved_0;
	float3 color;
    float reserved_1;
}

float RationalFunction(float x)
{
    float num = pow(x, rationalPowers[0]);
    float den = pow(x, rationalPowers[1]);
    float ratio = num / den;
    return abs(ratio);
    //return abs(pow(x, rationalPowers[0]) / pow(x, rationalPowers[1]));
}

float RationalFalloff(float x)
{
    float g = RationalFunction(falloffMax);
    float h = RationalFunction(x);
    float f = h - g;
    float s = scale;
    if (s == 0.0f)
        s = rcp(1 - g);
    
    return saturate(s * f);
}

struct PSIn
{
    float4 ssPos : SV_Position;
    float norm : NORMAL;
};

float4 main(PSIn psIn) : SV_Target
{
    float linearDistance = falloffMax * abs(psIn.norm);
    float4 output = float4(color, RationalFalloff(linearDistance));
    return float4(output);
}
