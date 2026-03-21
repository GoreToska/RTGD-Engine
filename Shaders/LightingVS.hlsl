struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
};

void main(in uint VertID : SV_VertexID, out VSOutput OUT)
{
    float2 uv = float2((VertID << 1) & 2, VertID & 2);
    OUT.UV       = uv;
    OUT.Position = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
}