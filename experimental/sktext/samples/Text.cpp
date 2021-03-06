// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Processor.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTime.h"
#include "samplecode/Sample.h"
#include "src/core/SkOSFile.h"
#include "src/shaders/SkColorShader.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"

using namespace skia::text;

namespace {
class TextSample_HelloWorld : public Sample {
protected:
    SkString name() override { return SkString("TextSample_HelloWorld"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Processor::drawText("Hello word", canvas, 0, 0);
    }

private:
    using INHERITED = Sample;
};

class TextSample_Align_Dir : public Sample {

public:
    TextSample_Align_Dir() : fUnicode(SkUnicode::Make()) { }
protected:
    SkString name() override { return SkString("TextSample_Align_Dir"); }

    void drawLine(SkCanvas* canvas, SkScalar w, SkScalar h,
                  const std::string& text,
                  TextAlign align,
                  TextDirection direction = TextDirection::kLtr) {
        SkColor background = SK_ColorGRAY;
        const std::u16string& ellipsis = u"\u2026";
        SkScalar margin = 20;

        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(SK_ColorWHITE);

        Processor::drawText(direction == TextDirection::kRtl ? mirror(text).c_str() : normal(text).c_str(),
                            canvas,
                            TextFormatStyle(align, direction),
                            SK_ColorBLACK, SK_ColorLTGRAY,
                            SkString("Roboto"), 12.0f, SkFontStyle::Normal(),
                            0, 0);
    }

    SkString mirror(const std::string& text) {
        std::u16string result;
        result += u"\u202E";
        //for (auto i = text.size(); i > 0; --i) {
        //  result += text[i - 1];
        //}
        for (auto ch : text) {
            result += ch;
        }
        result += u"\u202C";
        return fUnicode->convertUtf16ToUtf8(result);
    }

    SkString normal(const std::string& text) {
        std::u16string result;
        result += u"\u202D";
        for (auto ch : text) {
            result += ch;
        }
        result += u"\u202C";
        return fUnicode->convertUtf16ToUtf8(result);
    }

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorDKGRAY);
        SkScalar width = this->width() / 4;
        SkScalar height = this->height() / 2;

        const std::string line = "One line of text";

        drawLine(canvas, width, height, line, TextAlign::kLeft, TextDirection::kLtr);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kRight, TextDirection::kLtr);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kCenter, TextDirection::kLtr);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kJustify, TextDirection::kLtr);
        canvas->translate(-width * 3, height);

        drawLine(canvas, width, height, line, TextAlign::kLeft, TextDirection::kRtl);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kRight, TextDirection::kRtl);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kCenter, TextDirection::kRtl);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kJustify, TextDirection::kRtl);
        canvas->translate(width, 0);
    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};

class TextSample_LongLTR : public Sample {
protected:
    SkString name() override { return SkString("TextSample_LongLTR"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Processor::drawText("A very_very_very_very_very_very_very_very_very_very "
                "very_very_very_very_very_very_very_very_very_very very very very very very very "
                "very very very very very very very very very very very very very very very very "
                "very very very very very very very very very very very very very long text", canvas, this->width());

    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};

class TextSample_LongRTL : public Sample {
protected:
    SkString name() override { return SkString("TextSample_LongRTL"); }

    SkString mirror(const std::string& text) {
        std::u16string result;
        result += u"\u202E";
        for (auto i = text.size(); i > 0; --i) {
          result += text[i - 1];
        }
        for (auto ch : text) {
            result += ch;
        }
        result += u"\u202C";
        return fUnicode->convertUtf16ToUtf8(result);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Processor::drawText("LONG MIRRORED TEXT SHOULD SHOW RIGHT TO LEFT (AS NORMAL)", canvas, 0, 0);
    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};
}  // namespace

DEF_SAMPLE(return new TextSample_HelloWorld();)
DEF_SAMPLE(return new TextSample_Align_Dir();)
DEF_SAMPLE(return new TextSample_LongLTR();)
DEF_SAMPLE(return new TextSample_LongRTL();)
