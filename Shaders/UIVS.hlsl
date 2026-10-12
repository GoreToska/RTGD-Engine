cbuffer UIConstants{
    float4x4 g_Transform;
};

struct VSInput{
    float2 Position : ATTRIB0;
    float4 Color : ATTRIB1;
    float2 UV : ATTRIB2;
};

struct PSInput{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
};

void main(in VSInput IN, out PSInput OUT)
{
    OUT.Position = mul(float4(IN.Position, 0.0, 1.0), g_Transform);
    OUT.Color = IN.Color;
    OUT.UV = IN.UV;
}