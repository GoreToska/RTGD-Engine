Texture2D    g_Diffuse           : register(t0);
Texture2D    g_Normal           : register(t1); // normal map (tangent space)
Texture2D    g_MetallicRoughness: register(t2); // R=metallic, G=roughness
Texture2D    g_AO               : register(t3); 
SamplerState g_Sampler          : register(s0);

struct PSInput
{
    float4 Position      : SV_POSITION;
    float3 WorldPos      : WORLD_POSITION;
    float3 WorldNormal   : WORLD_NORMAL;
    float2 UV            : TEXCOORD;
    float3 WorldTangent  : TANGENT;
    float3 WorldBitangent: BITANGENT;
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

    OUT.Albedo = g_Diffuse.Sample(g_Sampler, IN.UV);

    // Normal mapping tangent space to world space
    float3 normalMap = g_Normal.Sample(g_Sampler, IN.UV).xyz * 2.0 - 1.0;

    float3x3 TBN = float3x3(
        normalize(IN.WorldTangent),
        normalize(IN.WorldBitangent),
        normalize(IN.WorldNormal)
    );
    float3 worldNormal = normalize(mul(normalMap, TBN));

    OUT.Normal = float4(worldNormal * 0.5 + 0.5, 1.0); // [0,1] 

    OUT.Position = float4(IN.WorldPos, 1.0);

    float2 metallicRoughness = g_MetallicRoughness.Sample(g_Sampler, IN.UV).rg;
    float  ao                = g_AO.Sample(g_Sampler, IN.UV).r;

    OUT.PBR = float4(
        metallicRoughness.r, // Metallic
        metallicRoughness.g, // Roughness
        ao,                  
        1.0
    );

    return OUT;
}