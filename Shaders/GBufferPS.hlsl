Texture2D    g_Diffuse           : register(t0);
Texture2D    g_Normal           : register(t1); // normal map (tangent space)
Texture2D    g_MetallicRoughness: register(t2); // R=metallic, G=roughness
Texture2D    g_AO               : register(t3); 
SamplerState g_Sampler          : register(s0);

struct PSInput
{
    float4 Position      : SV_POSITION;
    float3 WorldPos      : TEXCOORD0;
    float3 WorldNormal   : TEXCOORD1;
    float3 WorldTangent  : TEXCOORD2;
    float3 WorldBitangent: TEXCOORD3;
    float2 UV            : TEXCOORD4;
};

struct PSOutput
{
    float4 Albedo   : SV_TARGET0;
    float4 Normal   : SV_TARGET1;
    float4 Position : SV_TARGET2;
    float4 PBR      : SV_TARGET3; // R=metallic, G=roughness, B=AO
};

PSOutput main(in PSInput IN)
{
    PSOutput OUT;

    //float2 uv = float2(IN.UV.x, 1.0 - IN.UV.y);
    float2 uv = IN.UV;
    uv = float2(frac(uv.x), frac(uv.y));

    OUT.Albedo   = g_Diffuse.Sample(g_Sampler, uv);
    OUT.Position = float4(IN.WorldPos, 1.0);

    float3 N = normalize(IN.WorldNormal);
    float3 T = normalize(IN.WorldTangent);
    float3 B = normalize(IN.WorldBitangent);

    float3 normalSample = g_Normal.Sample(g_Sampler, uv).xyz * 2.0 - 1.0;

    T = normalize(T - dot(T, N) * N);
    B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);
    float3 worldNormal = normalize(mul(normalSample, TBN));

    OUT.Normal = float4(worldNormal * 0.5 + 0.5, 1.0);

    float2 mr = g_MetallicRoughness.Sample(g_Sampler, uv).rg;
    float  ao = g_AO.Sample(g_Sampler, uv).r;
    OUT.PBR   = float4(mr.r, mr.g, ao, 1.0);

    return OUT;
}