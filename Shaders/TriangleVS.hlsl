struct VSInput {
    float3 Position : ATTRIB0;
    float3 Color    : ATTRIB1;
};

struct PSInput {
    float4 Position : SV_POSITION;
    float3 Color    : COLOR;
};

void main(in VSInput IN, out PSInput OUT) {
    OUT.Position = float4(IN.Position, 1.0);
    OUT.Color    = IN.Color;
}