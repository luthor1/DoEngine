
cbuffer PerFrame : register(b0) {
    float4x4 MVP;
};

struct VS_Input {
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct PS_Input {
    float4 pos : SV_Position;
    float4 color : COLOR;
};

PS_Input vs_main(VS_Input input) {
    PS_Input output;
    // Apply MVP Matrix
    output.pos = mul(MVP, float4(input.pos, 1.0));
    output.color = input.color;
    return output;
}

float4 ps_main(PS_Input input) : SV_Target {
    return input.color;
}
