#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float4 b = float4(1.0);
    float4 c = float4(1.0, 1.0, 1.0, 1.0);
    if (any(b != c)) {
        _out.sk_FragColor = _uniforms.colorRed;
        return _out;
    } else {
        _out.sk_FragColor = _uniforms.colorGreen;
        return _out;
    }
    return _out;
}
