/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrSingleIntervalGradientColorizer.fp; do not modify.
 **************************************************************************************************/
#include "GrSingleIntervalGradientColorizer.h"

#include "src/core/SkUtils.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLSingleIntervalGradientColorizer : public GrGLSLFragmentProcessor {
public:
    GrGLSLSingleIntervalGradientColorizer() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrSingleIntervalGradientColorizer& _outer =
                args.fFp.cast<GrSingleIntervalGradientColorizer>();
        (void)_outer;
        auto start = _outer.start;
        (void)start;
        auto end = _outer.end;
        (void)end;
        startVar = args.fUniformHandler->addUniform(&_outer, kFragment_GrShaderFlag,
                                                    kHalf4_GrSLType, "start");
        endVar = args.fUniformHandler->addUniform(&_outer, kFragment_GrShaderFlag, kHalf4_GrSLType,
                                                  "end");
        fragBuilder->codeAppendf(
                R"SkSL(half t = half(%s.x);
%s = mix(%s, %s, t);
)SkSL",
                args.fSampleCoord, args.fOutputColor,
                args.fUniformHandler->getUniformCStr(startVar),
                args.fUniformHandler->getUniformCStr(endVar));
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrSingleIntervalGradientColorizer& _outer =
                _proc.cast<GrSingleIntervalGradientColorizer>();
        {
            const SkPMColor4f& startValue = _outer.start;
            if (startPrev != startValue) {
                startPrev = startValue;
                pdman.set4fv(startVar, 1, startValue.vec());
            }
            const SkPMColor4f& endValue = _outer.end;
            if (endPrev != endValue) {
                endPrev = endValue;
                pdman.set4fv(endVar, 1, endValue.vec());
            }
        }
    }
    SkPMColor4f startPrev = {SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN};
    SkPMColor4f endPrev = {SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN};
    UniformHandle startVar;
    UniformHandle endVar;
};
GrGLSLFragmentProcessor* GrSingleIntervalGradientColorizer::onCreateGLSLInstance() const {
    return new GrGLSLSingleIntervalGradientColorizer();
}
void GrSingleIntervalGradientColorizer::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                              GrProcessorKeyBuilder* b) const {}
bool GrSingleIntervalGradientColorizer::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSingleIntervalGradientColorizer& that = other.cast<GrSingleIntervalGradientColorizer>();
    (void)that;
    if (start != that.start) return false;
    if (end != that.end) return false;
    return true;
}
GrSingleIntervalGradientColorizer::GrSingleIntervalGradientColorizer(
        const GrSingleIntervalGradientColorizer& src)
        : INHERITED(kGrSingleIntervalGradientColorizer_ClassID, src.optimizationFlags())
        , start(src.start)
        , end(src.end) {
    this->cloneAndRegisterAllChildProcessors(src);
    this->setUsesSampleCoordsDirectly();
}
std::unique_ptr<GrFragmentProcessor> GrSingleIntervalGradientColorizer::clone() const {
    return std::make_unique<GrSingleIntervalGradientColorizer>(*this);
}
#ifdef SK_DEBUG
SkString GrSingleIntervalGradientColorizer::dumpInfo() const {
    return SkStringPrintf(
            "SingleIntervalGradientColorizer(start=half4(%f, %f, %f, %f), end=half4(%f, %f, %f, "
            "%f))",
            start.fR, start.fG, start.fB, start.fA, end.fR, end.fG, end.fB, end.fA);
}
#endif
