Texture2D    g_Source  : register(t0);
SamplerState g_Sampler : register(s0);

struct VSOutput {
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
};

float4 main(VSOutput IN) : SV_TARGET {
    return g_Source.Sample(g_Sampler, IN.UV);
}