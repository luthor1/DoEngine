struct VS_INPUT {
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};
struct PushConstants {
    float2 scale;
    float2 translate;
};
[[vk::push_constant]] PushConstants pc;

PS_INPUT vs_main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = float4(input.pos * pc.scale + pc.translate, 0, 1);
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

Texture2D    tTexture : register(t0);
SamplerState sSampler : register(s0);

float4 ps_main(PS_INPUT input) : SV_Target {
    float4 color = input.col * tTexture.Sample(sSampler, input.uv);
    return color;
}
