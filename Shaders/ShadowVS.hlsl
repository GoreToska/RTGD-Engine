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

cbuffer ShadowConstants : register(b2)
{
    float4x4 g_LightViewProjection[4];
    float4 g_CascadeSplits;
    float4 g_AtlasRects[4];
    float4 g_ShadowParams;
};

struct VSInput
{
    float3 Position : ATTRIB0;
    float3 Normal : ATTRIB1;
    float4 Tangent : ATTRIB2;
    float2 UV : ATTRIB3;
};

void main(in VSInput IN, out float4 OUT : SV_POSITION)
{
    float4 worldPos = mul(float4(IN.Position, 1.0), g_Model);
    OUT = mul(worldPos, g_LightViewProjection[0]);
};