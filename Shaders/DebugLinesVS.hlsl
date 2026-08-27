cbuffer CameraConstants : register(b0)
{
    float4x4 g_View;
    float4x4 g_Proj;
    float4   g_CameraPos;
};

struct VSInput
{
    float3 Position : ATTRIB0;
    float4 Color    : ATTRIB1;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color    : TEXCOORD0;
};

void main(in VSInput IN, out PSInput OUT)
{
    float4 viewPos = mul(float4(IN.Position, 1.0), g_View);
    OUT.Position   = mul(viewPos, g_Proj);
    OUT.Color      = IN.Color;
}
