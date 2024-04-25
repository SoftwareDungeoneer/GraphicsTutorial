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
    //float2 u = end - begin;
    //float2 n = normalize(float2(-u.y, u.x));
    //float2 v = psIn.ssPos.xy - begin;
    //float2 proj_v_n = dot(v, n) * n;
    //float alpha = RationalFalloff(length(proj_v_n));
    
    //return float4(1, 0, 0, alpha);
    
    float linearDistance = 5 * abs(psIn.norm);
    float4 output = float4(1, 0, 0, RationalFalloff(linearDistance));
    return float4(output);

}
