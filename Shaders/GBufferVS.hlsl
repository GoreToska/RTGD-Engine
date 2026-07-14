cbuffer CameraConstants : register(b0)
{
    float4x4 g_View;
    float4x4 g_Proj;
    float4   g_CameraPos;
};

cbuffer ObjectConstants : register(b1)
{
    float4x4 g_Model;

#if RTGD_EDITOR
    uint g_EntityID;
    uint _objPad0;
    uint _objPad1;
    uint _objPad2;
#endif
};

struct VSInput
{
    float3 Position : ATTRIB0;
    float3 Normal   : ATTRIB1;
    float4 Tangent  : ATTRIB2;
    float2 UV       : ATTRIB3;
};

struct PSInput
{
    float4 Position      : SV_POSITION;
    float3 WorldPos      : TEXCOORD0;
    float3 WorldNormal   : TEXCOORD1;
    float3 WorldTangent  : TEXCOORD2;
    float3 WorldBitangent: TEXCOORD3;
    float2 UV            : TEXCOORD4;
#if RTGD_EDITOR
    nointerpolation uint EntityID : TEXCOORD5;
#endif
};

void main(in VSInput IN, out PSInput OUT)
{
    float4 worldPos = mul(float4(IN.Position, 1.0), g_Model);
    float4 viewPos  = mul(worldPos, g_View);
    OUT.Position    = mul(viewPos,  g_Proj);
    OUT.WorldPos    = worldPos.xyz;
    OUT.UV          = IN.UV;

    float3x3 normalMatrix = (float3x3)g_Model;

    float3 N = normalize(mul(IN.Normal,        normalMatrix));
    float3 T = normalize(mul(IN.Tangent.xyz,   normalMatrix));

    T = normalize(T - dot(T, N) * N);

    float3 B = cross(N, T) * IN.Tangent.w;

    OUT.WorldNormal    = N;
    OUT.WorldTangent   = T;
    OUT.WorldBitangent = B;

#if RTGD_EDITOR
    OUT.EntityID = g_EntityID;
#endif
}