/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleMeshDrawOpHelper_DEFINED
#define GrSimpleMeshDrawOpHelper_DEFINED

#include "GrAppliedClip.h"
#include "GrOpFlushState.h"
#include "GrPipeline.h"
#include "GrProcessorSet.h"
#include "GrRect.h"
#include "GrUserStencilSettings.h"

/**
 * This class can be used to help implement simple mesh draw ops. It reduces the amount of
 * boilerplate code to type and also provides a mechanism for optionally allocating space for a
 * GrProcessorSet based on a GrPaint. It is intended to be used by ops that construct a single
 * GrPipeline for a uniform primitive color and a GrPaint.
 */
class GrSimpleMeshDrawOpHelper {
public:
    struct MakeArgs;

    /**
     * This can be used by a Op class to perform allocation and initialization such that a
     * GrProcessorSet (if required) is allocated at the same time as the Op instance. It requires
     * that Op implements a constructor of the form:
     *      Op(MakeArgs, GrColor, OpArgs...)
     * which is public or made accessible via 'friend'.
     */
    template <typename Op, typename... OpArgs>
    static std::unique_ptr<GrDrawOp> FactoryHelper(GrPaint&& paint, OpArgs... opArgs);

    enum class Flags : uint32_t {
        kNone = 0x0,
        kSnapVerticesToPixelCenters = 0x1,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags);

    GrSimpleMeshDrawOpHelper(const MakeArgs& args, GrAAType aaType, Flags flags = Flags::kNone)
            : fProcessors(args.fProcessorSet)
            , fPipelineFlags(args.fSRGBFlags)
            , fAAType((int)aaType)
            , fRequiresDstTexture(false)
            , fUsesLocalCoords(false)
            , fCompatibleWithAlphaAsCoveage(false) {
        SkDEBUGCODE(fDidAnalysis = false);
        if (GrAATypeIsHW(aaType)) {
            fPipelineFlags |= GrPipeline::kHWAntialias_Flag;
        }
        if (flags & Flags::kSnapVerticesToPixelCenters) {
            fPipelineFlags |= GrPipeline::kSnapVerticesToPixelCenters_Flag;
        }
    }

    ~GrSimpleMeshDrawOpHelper() {
        if (fProcessors) {
            fProcessors->~GrProcessorSet();
        }
    }

    GrSimpleMeshDrawOpHelper() = delete;
    GrSimpleMeshDrawOpHelper(const GrSimpleMeshDrawOpHelper&) = delete;
    GrSimpleMeshDrawOpHelper& operator=(const GrSimpleMeshDrawOpHelper&) = delete;

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const {
        return GrAATypeIsHW((this->aaType())) ? GrDrawOp::FixedFunctionFlags::kUsesHWAA
                                              : GrDrawOp::FixedFunctionFlags::kNone;
    }

    bool isCompatible(const GrSimpleMeshDrawOpHelper& that, const GrCaps& caps,
                      const SkRect& aBounds, const SkRect& bBounds) const {
        if (SkToBool(fProcessors) != SkToBool(that.fProcessors)) {
            return false;
        }
        if (fProcessors) {
            if (*fProcessors != *that.fProcessors) {
                return false;
            }
            if (fRequiresDstTexture || (fProcessors->xferProcessor() &&
                                        fProcessors->xferProcessor()->xferBarrierType(caps))) {
                if (GrRectsTouchOrOverlap(aBounds, bBounds)) {
                    return false;
                }
            }
        }
        bool result = fPipelineFlags == that.fPipelineFlags && fAAType == that.fAAType;
        SkASSERT(!result || fCompatibleWithAlphaAsCoveage == that.fCompatibleWithAlphaAsCoveage);
        SkASSERT(!result || fUsesLocalCoords == that.fUsesLocalCoords);
        return result;
    }

    GrDrawOp::RequiresDstTexture xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip,
                                                      GrProcessorAnalysisCoverage geometryCoverage,
                                                      GrColor* color) {
        SkDEBUGCODE(fDidAnalysis = true);
        GrProcessorSet::Analysis analysis;
        if (fProcessors) {
            GrProcessorAnalysisCoverage coverage = geometryCoverage;
            if (GrProcessorAnalysisCoverage::kNone == coverage) {
                coverage = clip->clipCoverageFragmentProcessor()
                                   ? GrProcessorAnalysisCoverage::kSingleChannel
                                   : GrProcessorAnalysisCoverage::kNone;
            }
            bool isMixedSamples = this->aaType() == GrAAType::kMixedSamples;
            analysis = fProcessors->finalize(*color, coverage, clip, isMixedSamples, caps, color);
        } else {
            analysis = GrProcessorSet::EmptySetAnalysis();
        }
        fRequiresDstTexture = analysis.requiresDstTexture();
        fUsesLocalCoords = analysis.usesLocalCoords();
        fCompatibleWithAlphaAsCoveage = analysis.isCompatibleWithCoverageAsAlpha();
        return analysis.requiresDstTexture() ? GrDrawOp::RequiresDstTexture::kYes
                                             : GrDrawOp::RequiresDstTexture::kNo;
    }

    bool usesLocalCoords() const {
        SkASSERT(fDidAnalysis);
        return fUsesLocalCoords;
    }

    bool compatibleWithAlphaAsCoverage() const { return fCompatibleWithAlphaAsCoveage; }

    GrPipeline* makePipeline(GrMeshDrawOp::Target* target) const {
        return target->allocPipeline(this->pipelineInitArgs(target));
    }

    struct MakeArgs {
    private:
        MakeArgs() = default;

        GrProcessorSet* fProcessorSet;
        uint32_t fSRGBFlags;

        friend class GrSimpleMeshDrawOpHelper;
    };

    SkString dumpInfo() const {
        SkString result = this->processors().dumpProcessors();
        result.append("AA Type: ");
        switch (this->aaType()) {
            case GrAAType::kNone:
                result.append(" none\n");
                break;
            case GrAAType::kCoverage:
                result.append(" coverage\n");
                break;
            case GrAAType::kMSAA:
                result.append(" msaa\n");
                break;
            case GrAAType::kMixedSamples:
                result.append(" mixed samples\n");
                break;
        }
        result.append(GrPipeline::DumpFlags(fPipelineFlags));
        return result;
    }

protected:
    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }
    uint32_t pipelineFlags() const { return fPipelineFlags; }
    const GrProcessorSet& processors() const {
        return fProcessors ? *fProcessors : GrProcessorSet::EmptySet();
    }

    GrPipeline::InitArgs pipelineInitArgs(GrMeshDrawOp::Target* target) const {
        GrPipeline::InitArgs args;
        args.fFlags = this->pipelineFlags();
        args.fProcessors = &this->processors();
        args.fRenderTarget = target->renderTarget();
        args.fAppliedClip = target->clip();
        args.fDstProxy = target->dstProxy();
        args.fCaps = &target->caps();
        args.fResourceProvider = target->resourceProvider();
        return args;
    }

private:
    GrProcessorSet* fProcessors;
    unsigned fPipelineFlags : 8;
    unsigned fAAType : 2;
    unsigned fRequiresDstTexture : 1;
    unsigned fUsesLocalCoords : 1;
    unsigned fCompatibleWithAlphaAsCoveage : 1;
    SkDEBUGCODE(unsigned fDidAnalysis : 1;)
};

/**
 * This class extends GrSimpleMeshDrawOpHelper to support an optional GrUserStencilSettings. This
 * uses private inheritance because it non-virtually overrides methods in the base class and should
 * never be used with a GrSimpleMeshDrawOpHelper pointer or reference.
 */
class GrSimpleMeshDrawOpHelperWithStencil : private GrSimpleMeshDrawOpHelper {
public:
    using MakeArgs = GrSimpleMeshDrawOpHelper::MakeArgs;
    using Flags = GrSimpleMeshDrawOpHelper::Flags;

    // using declarations can't be templated, so this is a pass through function instead.
    template <typename Op, typename... OpArgs>
    static std::unique_ptr<GrDrawOp> FactoryHelper(GrPaint&& paint, OpArgs... opArgs) {
        return GrSimpleMeshDrawOpHelper::FactoryHelper<Op, OpArgs...>(
                std::move(paint), std::forward<OpArgs>(opArgs)...);
    }

    GrSimpleMeshDrawOpHelperWithStencil(const MakeArgs& args, GrAAType aaType,
                                        const GrUserStencilSettings* stencilSettings,
                                        Flags flags = Flags::kNone)
            : INHERITED(args, aaType, flags)
            , fStencilSettings(stencilSettings ? stencilSettings
                                               : &GrUserStencilSettings::kUnused) {}

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const {
        GrDrawOp::FixedFunctionFlags flags = INHERITED::fixedFunctionFlags();
        if (fStencilSettings != &GrUserStencilSettings::kUnused) {
            flags |= GrDrawOp::FixedFunctionFlags::kUsesStencil;
        }
        return flags;
    }

    using GrSimpleMeshDrawOpHelper::xpRequiresDstTexture;
    using GrSimpleMeshDrawOpHelper::usesLocalCoords;
    using GrSimpleMeshDrawOpHelper::compatibleWithAlphaAsCoverage;

    bool isCompatible(const GrSimpleMeshDrawOpHelperWithStencil& that, const GrCaps& caps,
                      const SkRect& aBounds, const SkRect& bBounds) const {
        return INHERITED::isCompatible(that, caps, aBounds, bBounds) &&
               fStencilSettings == that.fStencilSettings;
    }

    const GrPipeline* makePipeline(GrMeshDrawOp::Target* target) const {
        auto args = INHERITED::pipelineInitArgs(target);
        args.fUserStencil = fStencilSettings;
        return target->allocPipeline(args);
    }

    SkString dumpInfo() const {
        SkString result = INHERITED::dumpInfo();
        result.appendf("Stencil settings: %s\n", (fStencilSettings ? "yes" : "no"));
        return result;
    }

private:
    const GrUserStencilSettings* fStencilSettings;
    typedef GrSimpleMeshDrawOpHelper INHERITED;
};

template <typename Op, typename... OpArgs>
std::unique_ptr<GrDrawOp> GrSimpleMeshDrawOpHelper::FactoryHelper(GrPaint&& paint,
                                                                  OpArgs... opArgs) {
    MakeArgs makeArgs;
    makeArgs.fSRGBFlags = GrPipeline::SRGBFlagsFromPaint(paint);
    GrColor color = paint.getColor();
    if (paint.isTrivial()) {
        makeArgs.fProcessorSet = nullptr;
        return std::unique_ptr<GrDrawOp>(new Op(makeArgs, color, std::forward<OpArgs>(opArgs)...));
    } else {
        char* mem = (char*)GrOp::operator new(sizeof(Op) + sizeof(GrProcessorSet));
        char* setMem = mem + sizeof(Op);
        makeArgs.fProcessorSet = new (setMem) GrProcessorSet(std::move(paint));
        return std::unique_ptr<GrDrawOp>(
                new (mem) Op(makeArgs, color, std::forward<OpArgs>(opArgs)...));
    }
}

GR_MAKE_BITFIELD_CLASS_OPS(GrSimpleMeshDrawOpHelper::Flags)

#endif
