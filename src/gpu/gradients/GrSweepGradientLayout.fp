/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

layout(tracked) in uniform half bias;
layout(tracked) in uniform half scale;

half4 main(float2 coord) {
    // On some devices they incorrectly implement atan2(y,x) as atan(y/x). In actuality it is
    // atan2(y,x) = 2 * atan(y / (sqrt(x^2 + y^2) + x)). So to work around this we pass in (sqrt(x^2
    // + y^2) + x) as the second parameter to atan2 in these cases. We let the device handle the
    // undefined behavior of the second paramenter being 0 instead of doing the divide ourselves and
    // using atan instead.
    half angle = sk_Caps.atan2ImplementedAsAtanYOverX
            ? half(2 * atan(-coord.y, length(coord) - coord.x))
            : half(atan(-coord.y, -coord.x));

    // 0.1591549430918 is 1/(2*pi), used since atan returns values [-pi, pi]
    half t = (angle * 0.1591549430918 + 0.5 + bias) * scale;
    return half4(t, 1, 0, 0); // y = 1 for always valid
}

//////////////////////////////////////////////////////////////////////////////

@header {
    #include "src/gpu/effects/GrMatrixEffect.h"
    #include "src/gpu/gradients/GrGradientShader.h"
    #include "src/shaders/gradients/SkSweepGradient.h"
}

// The sweep gradient never rejects a pixel so it doesn't change opacity
@optimizationFlags {
    kPreservesOpaqueInput_OptimizationFlag
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(const SkSweepGradient& gradient,
                                                     const GrFPArgs& args);
}

@cppEnd {
    std::unique_ptr<GrFragmentProcessor> GrSweepGradientLayout::Make(
            const SkSweepGradient& grad, const GrFPArgs& args) {
        SkMatrix matrix;
        if (!grad.totalLocalMatrix(args.fPreLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }
        matrix.postConcat(grad.getGradientMatrix());
        return GrMatrixEffect::Make(
                matrix, std::unique_ptr<GrFragmentProcessor>(new GrSweepGradientLayout(
                        grad.getTBias(), grad.getTScale())));
    }
}

//////////////////////////////////////////////////////////////////////////////

@test(d) {
    SkScalar scale = GrGradientShader::RandomParams::kGradientScale;
    SkPoint center;
    center.fX = d->fRandom->nextRangeScalar(0.0f, scale);
    center.fY = d->fRandom->nextRangeScalar(0.0f, scale);

    GrGradientShader::RandomParams params(d->fRandom);
    auto shader = params.fUseColors4f ?
        SkGradientShader::MakeSweep(center.fX, center.fY, params.fColors4f, params.fColorSpace,
                                    params.fStops, params.fColorCount) :
        SkGradientShader::MakeSweep(center.fX, center.fY,  params.fColors,
                                    params.fStops, params.fColorCount);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());
    SkASSERT_RELEASE(fp);
    return fp;
}
