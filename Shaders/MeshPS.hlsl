struct PSInput {
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD;
};

float4 main(in PSInput IN) : SV_TARGET
{
    float3 lightDir = normalize(float3(1.0, 1.0, -1.0));
    float  diffuse  = max(dot(normalize(IN.Normal), lightDir), 0.1);
    return float4(diffuse, diffuse, diffuse, 1.0);
}