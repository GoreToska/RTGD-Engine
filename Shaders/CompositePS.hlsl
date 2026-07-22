Texture2D g_SceneColor : register(t0);
SamplerState g_Sampler : register(s0);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

float4 main(in PSInput IN) : SV_TARGET
{
    return g_SceneColor.Sample(g_Sampler, IN.UV);
}