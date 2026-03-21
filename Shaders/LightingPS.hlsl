Texture2D    g_Diffuse  : register(t0);
Texture2D    g_Normal   : register(t1);
Texture2D    g_Position : register(t2);
Texture2D    g_PBR      : register(t3);
SamplerState g_Sampler  : register(s0);

static const uint MAX_DIRECTIONAL_LIGHTS = 4;
static const uint MAX_POINT_LIGHTS       = 64;
static const uint MAX_SPOT_LIGHTS        = 16;
static const float PI                    = 3.14159265359;

struct DirectionalLightData
{
    float3 Direction; float Intensity;
    float3 Color;     float _pad;
};

struct PointLightData
{
    float3 Position; float Radius;
    float3 Color;    float Intensity;
};

struct SpotLightData
{
    float3 Position;  float InnerAngle;
    float3 Direction; float OuterAngle;
    float3 Color;     float Intensity;
    float  Radius;    float3 _pad;
};

cbuffer LightConstants : register(b0)
{
    float3 g_AmbientColor;     float g_AmbientIntensity;
    uint   g_DirectionalCount; uint g_PointCount; uint g_SpotCount; float _pad;
    DirectionalLightData g_DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
    PointLightData       g_PointLights[MAX_POINT_LIGHTS];
    SpotLightData        g_SpotLights[MAX_SPOT_LIGHTS];
};

cbuffer CameraConstants : register(b1)
{
    float4x4 g_View;
    float4x4 g_Proj;
    float4             g_CameraPos;
};


// Normal Distribution Function — GGX/Trowbridge-Reitz
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom       = PI * denom * denom;

    return num / max(denom, 0.0001);
}

// Geometry Function — Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    return NdotV / max(NdotV * (1.0 - k) + k, 0.0001);
}

// Smith's method
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness)
         * GeometrySchlickGGX(NdotL, roughness);
}

// Fresnel — Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


float3 CalcPBR(float3 albedo, float metallic, float roughness,
               float3 N, float3 V, float3 L, float3 lightColor, float attenuation)
{
    float3 H  = normalize(V + L);
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    // Cook-Torrance BRDF
    float  NDF = DistributionGGX(N, H, roughness);
    float  G   = GeometrySmith(N, V, L, roughness);
    float3 F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 numerator   = NDF * G * F;
    float  denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    float3 specular    = numerator / max(denominator, 0.001);

    // kS = Fresnel, kD = diffuse
    float3 kS = F;
    float3 kD = (float3(1.0, 1.0, 1.0) - kS) * (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);

    return (kD * albedo / PI + specular) * lightColor * NdotL * attenuation;
}


float3 CalcDirectionalPBR(DirectionalLightData light, float3 albedo,
                           float metallic, float roughness,
                           float3 N, float3 V)
{
    float3 L = normalize(-light.Direction);
    return CalcPBR(albedo, metallic, roughness, N, V, L,
                   light.Color * light.Intensity, 1.0);
}

float3 CalcPointPBR(PointLightData light, float3 worldPos, float3 albedo,
                    float metallic, float roughness, float3 N, float3 V)
{
    float3 delta = light.Position - worldPos;
    float  dist  = length(delta);
    if (dist > light.Radius) return float3(0, 0, 0);

    float3 L           = normalize(delta);
    float  attenuation = 1.0 - smoothstep(0.0, light.Radius, dist);

    return CalcPBR(albedo, metallic, roughness, N, V, L,
                   light.Color * light.Intensity, attenuation);
}

float3 CalcSpotPBR(SpotLightData light, float3 worldPos, float3 albedo,
                   float metallic, float roughness, float3 N, float3 V)
{
    float3 delta = light.Position - worldPos;
    float  dist  = length(delta);
    if (dist > light.Radius) return float3(0, 0, 0);

    float3 L         = normalize(delta);
    float  theta     = dot(L, normalize(-light.Direction));
    float  epsilon   = light.InnerAngle - light.OuterAngle;
    float  spotFactor = clamp((theta - light.OuterAngle) / epsilon, 0.0, 1.0);
    if (spotFactor <= 0.0) return float3(0, 0, 0);

    float attenuation = (1.0 - smoothstep(0.0, light.Radius, dist)) * spotFactor;

    return CalcPBR(albedo, metallic, roughness, N, V, L,
                   light.Color * light.Intensity, attenuation);
}


struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
};

float4 main(in PSInput IN) : SV_TARGET
{
    float4 albedoSample = g_Diffuse  .Sample(g_Sampler, IN.UV);
    float3 albedo       = albedoSample.rgb;
    float3 normalEnc    = g_Normal  .Sample(g_Sampler, IN.UV).xyz;
    float3 worldPos     = g_Position.Sample(g_Sampler, IN.UV).xyz;
    float4 pbrSample    = g_PBR     .Sample(g_Sampler, IN.UV);

    float metallic  = pbrSample.r;
    float roughness = max(pbrSample.g, 0.04);
    float ao        = pbrSample.b;

    float3 N = normalize(normalEnc * 2.0 - 1.0);

    //return albedo;
    //return float4(N * 0.5 + 0.5, 1.0);

    if (dot(N, N) < 0.01)
        return float4(0.0, 0.0, 0.0, 1.0);

    float3 V = normalize(g_CameraPos.xyz - worldPos);

    // Ambient
    float3 F0      = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float3 ambient = g_AmbientColor * g_AmbientIntensity * albedo * ao;

    float3 Lo = float3(0.0, 0.0, 0.0);

    // Directional lights
    for (uint d = 0; d < g_DirectionalCount; d++)
        Lo += CalcDirectionalPBR(g_DirectionalLights[d],
                                  albedo, metallic, roughness, N, V);

    // Point lights
    for (uint p = 0; p < g_PointCount; p++)
        Lo += CalcPointPBR(g_PointLights[p], worldPos,
                            albedo, metallic, roughness, N, V);

    // Spot lights
    for (uint s = 0; s < g_SpotCount; s++)
        Lo += CalcSpotPBR(g_SpotLights[s], worldPos,
                           albedo, metallic, roughness, N, V);

    float3 color = ambient + Lo;

    // Tone mapping — Reinhard
    color = color / (color + float3(1.0, 1.0, 1.0));

    // Gamma correction
    color = pow(color, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));

    return float4(color, 1.0);
}