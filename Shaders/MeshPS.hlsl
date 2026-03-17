Texture2D    g_Texture : register(t0);
SamplerState g_Sampler : register(s0); 

struct PSInput {
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD;
};

float4 main(in PSInput IN) : SV_TARGET
{
    float4 texColor = g_Texture.Sample(g_Sampler, IN.UV);
    float3 lightDir = normalize(float3(1.0, 1.0, -1.0));
    float  diffuse  = max(dot(normalize(IN.Normal), lightDir), 0.1);
    return float4(texColor.rgb * diffuse, texColor.a);
}