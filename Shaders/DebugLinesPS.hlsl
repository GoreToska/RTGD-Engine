struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color    : TEXCOORD0;
};

float4 main(in PSInput IN) : SV_TARGET
{
    return IN.Color;
}
