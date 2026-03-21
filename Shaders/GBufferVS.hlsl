cbuffer CameraConstants : register(b0)
{
    float4x4 g_View;
    float4x4 g_Proj;
    float4             g_CameraPos;
};

cbuffer ObjectConstants : register(b1)
{
    float4x4 g_Model;
};

struct VSInput
{
    float3 Position : ATTRIB0;
    float3 Normal   : ATTRIB1;
    float2 UV       : ATTRIB2;
};

struct PSInput
{
    float4 Position    : SV_POSITION;
    float3 WorldPos    : WORLD_POSITION;
    float3 WorldNormal : WORLD_NORMAL;
    float2 UV          : TEXCOORD;
    float3 WorldTangent : TANGENT;
    float3 WorldBitangent : BITANGENT;
};

void main(in VSInput IN, out PSInput OUT)
{
    float4 worldPos = mul(float4(IN.Position, 1.0), g_Model);
    float4 viewPos  = mul(worldPos, g_View);

    OUT.Position      = mul(viewPos, g_Proj);
    OUT.WorldPos      = worldPos.xyz;
    OUT.WorldNormal   = normalize(mul(float4(IN.Normal, 0.0), g_Model).xyz);
    OUT.UV            = IN.UV;


    float3 up      = abs(OUT.WorldNormal.y) < 0.999 ? float3(0,1,0) : float3(1,0,0);
    OUT.WorldTangent   = normalize(cross(up, OUT.WorldNormal));
    OUT.WorldBitangent = cross(OUT.WorldNormal, OUT.WorldTangent);
}