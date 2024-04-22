cbuffer LineParams
{
    float2 begin;
    float2 end;
    uint falloffFunction;
    float falloffMax;
    float scale;
    float clip;
    float2 rationalPowers;
}

float RationalFunction(float x)
{
    return abs(pow(x, rationalPowers[0]) / pow(x, rationalPowers[1]));

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
};

float4 main(PSIn psIn) : SV_Target
{
    float2 u = end - begin;
    float2 n = normalize(float2(-u.y, u.x));
    float2 v = psIn.ssPos.xy - begin;
    float theta = dot(normalize(v), n);
    float dist = n * theta;
    
    return float4(1, 1, 1, RationalFalloff(dist));
}
