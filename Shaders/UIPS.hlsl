Texture2D g_Texture;
SamplerState g_Sampler;

struct PSInput{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
};

float3 SRGBToLinear(float3 c)
{
    return lerp(c / 12.92, pow((c + 0.055) / 1.055, 2.4), step(0.04045, c));
}

float4 main(in PSInput IN) : SV_TARGET
{
    float4 c = IN.Color * g_Texture.Sample(g_Sampler, IN.UV);
    if(c.a > 0.0)
        c.rgb = SRGBToLinear(c.rgb / c.a) * c.a;
    return c;
}