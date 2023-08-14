#pragma once

#include <karm-base/cons.h>
#include <karm-base/opt.h>
#include <karm-logger/logger.h>

#include "../bscan.h"

namespace Ttf {

// https://learn.microsoft.com/en-us/typography/opentype/spec/chapter2#language-system-table
struct LangSys : public BChunk {
    using LookupOrderOffset = BField<u16be, 0>;
    using ReqFeatureIndex = BField<u16be, 2>;
    using FeatureCount = BField<u16be, 4>;

    Str tag;

    LangSys(Str tag, Bytes bytes)
        : BChunk{bytes}, tag(tag) {}

    auto iterFeatures() const {
        auto s = begin().skip(6);
        return range((usize)get<FeatureCount>())
            .map([s](auto) mutable {
                return s.nextU16be();
            });
    }
};

// https://learn.microsoft.com/en-us/typography/opentype/spec/chapter2#script-list-table-and-script-record
struct ScriptTable : public BChunk {
    using DefaultLangSysOffset = BField<u16be, 0>;
    using LangSysCount = BField<u16be, 2>;

    Str tag;

    ScriptTable(Str tag, Bytes bytes)
        : BChunk{bytes}, tag(tag) {}

    LangSys defaultLangSys() const {
        return {"DFLT", begin().skip(get<DefaultLangSysOffset>()).restBytes()};
    }

    usize len() const {
        return get<LangSysCount>();
    }

    LangSys at(usize i) const {
        auto s = begin().skip(4 + i * 6);
        return {s.nextStr(4), begin().skip(s.nextU16be()).restBytes()};
    }

    auto iter() const {
        return ::iter(*this);
    }
};

// https://learn.microsoft.com/en-us/typography/opentype/spec/chapter2#script-list-table-and-script-record
struct ScriptList : public BChunk {
    using ScriptCount = BField<u16be, 0>;

    usize len() const {
        return get<ScriptCount>();
    }

    ScriptTable at(usize i) const {
        auto s = begin().skip(2 + i * 6);
        auto tag = s.nextStr(4);
        auto off = s.nextU16be();
        return ScriptTable{tag, begin().skip(off).restBytes()};
    }

    auto iter() const {
        return ::iter(*this);
    }

    Res<ScriptTable> lookup(Str tag) {
        for (auto const &script : iter()) {
            if (Op::eq(script.tag, tag)) {
                return Ok(script);
            }
        }

        return Error::invalidInput("script tag not found");
    }
};

// https://learn.microsoft.com/en-us/typography/opentype/spec/chapter2#feature-table
struct FeatureTable : public BChunk {
    using FeatureParamsOffset = BField<u16be, 0>;
    using LookupCount = BField<u16be, 2>;

    Str tag;

    FeatureTable(Str tag, Bytes bytes)
        : BChunk{bytes}, tag(tag) {}

    auto iterLookups() const {
        auto s = begin().skip(4);
        return range((usize)get<LookupCount>())
            .map([s](auto) mutable {
                return s.nextU16be();
            });
    }
};

// https://learn.microsoft.com/en-us/typography/opentype/spec/chapter2#feature-list-table
struct FeatureList : public BChunk {
    using featureCount = BField<u16be, 0>;

    usize len() const {
        return get<featureCount>();
    }

    FeatureTable at(usize i) const {
        auto s = begin().skip(2 + i * 6);
        auto tag = s.nextStr(4);
        auto off = s.nextU16be();
        return FeatureTable{tag, begin().skip(off).restBytes()};
    }

    auto iter() const {
        return ::iter(*this);
    }
};

// https://learn.microsoft.com/en-us/typography/opentype/spec/chapter2#coverage-table
struct CoverageTable : public BChunk {
    using Format = BField<u16be, 0>;
    using GlyphCount = BField<u16be, 2>;

    usize format() const { return get<Format>(); }

    usize len() const { return get<GlyphCount>(); }

    Opt<usize> coverageIndex(usize glyphId) {
        auto s = begin().skip(4);

        if (format() == 1) {
            for (auto i : range(len())) {
                auto glyph = s.nextU16be();
                if (glyph == glyphId) {
                    return i;
                }
            }
        }

        if (format() == 2) {
            for (auto i : range(len())) {
                (void)i;
                auto start = s.nextU16be();
                auto end = s.nextU16be();
                auto index = s.nextU16be();
                if (start <= glyphId and glyphId <= end) {
                    return index + glyphId - start;
                }
            }
        }

        return NONE;
    }
};

struct LookupSubtableBase : public BChunk {
    using Format = BField<u16be, 0>;

    u16 format() const { return get<Format>(); }
};

struct ValueRecord {
    i16 xPlacement;
    i16 yPlacement;
    i16 xAdvance;
    i16 yAdvance;
    i16 xPlacementDeviceOffset;
    i16 yPlacementDeviceOffset;
    i16 xAdvanceDeviceOffset;
    i16 yAdvanceDeviceOffset;

    enum Format : u16 {
        X_PLACEMENT = 1 << 0,
        Y_PLACEMENT = 1 << 1,
        X_ADVANCE = 1 << 2,
        Y_ADVANCE = 1 << 3,
        X_PLACEMENT_DEVICE = 1 << 4,
        Y_PLACEMENT_DEVICE = 1 << 5,
        X_ADVANCE_DEVICE = 1 << 6,
        Y_ADVANCE_DEVICE = 1 << 7,
    };

    static ValueRecord read(BScan &s, u16 format) {
        ValueRecord r = {};

        if (format & X_PLACEMENT)
            r.xPlacement = s.nextI16be();

        if (format & Y_PLACEMENT)
            r.yPlacement = s.nextI16be();

        if (format & X_ADVANCE)
            r.xAdvance = s.nextI16be();

        if (format & Y_ADVANCE)
            r.yAdvance = s.nextI16be();

        if (format & X_PLACEMENT_DEVICE)
            r.xPlacementDeviceOffset = s.nextI16be();

        if (format & Y_PLACEMENT_DEVICE)
            r.yPlacementDeviceOffset = s.nextI16be();

        if (format & X_ADVANCE_DEVICE)
            r.xAdvanceDeviceOffset = s.nextI16be();

        if (format & Y_ADVANCE_DEVICE)
            r.yAdvanceDeviceOffset = s.nextI16be();

        return r;
    }

    static usize len(usize format) {
        return popcount(format & 0xff) * sizeof(i16);
    }
};

struct GlyphPairAdjustment : public LookupSubtableBase {
    static constexpr int FORMAT = 1;

    Opt<Pair<ValueRecord>> adjustments(usize prev, usize curr) {
        auto s = begin();

        // Read the table header
        /* format = */ s.nextU16be();
        auto coverageOffset = s.nextU16be();
        auto valueFormat1 = s.nextU16be();
        auto valueFormat2 = s.nextU16be();
        auto pairSetCount = s.nextU16be();

        // Lookup the coverage index for the first glyph
        CoverageTable coverage{begin().skip(coverageOffset).restBytes()};
        auto coverageIndex = coverage.coverageIndex(prev);

        if (not coverageIndex or (*coverageIndex >= pairSetCount))
            return NONE;

        auto value1len = ValueRecord::len(valueFormat1);
        auto value2len = ValueRecord::len(valueFormat2);
        auto pairSetOffset = s.skip(coverageIndex.unwrap() * 2).nextU16be();

        // Lookup the PairSet table for the second glyph
        auto pairSetTable = begin().skip(pairSetOffset);
        auto pairValueCount = pairSetTable.nextU16be();

        for (usize i : range(pairValueCount)) {
            (void)i;
            auto secondGlyph = pairSetTable.nextU16be();
            if (secondGlyph == curr) {
                ValueRecord value1 = ValueRecord::read(pairSetTable, valueFormat1);
                ValueRecord value2 = ValueRecord::read(pairSetTable, valueFormat2);
                return Pair<ValueRecord>{value1, value2};
            }

            pairSetTable.skip(value1len + value2len);
        }

        return NONE;
    }
};

// https://learn.microsoft.com/en-us/typography/opentype/spec/chapter2#class-definition-table
struct ClassDef : public BChunk {
    Opt<usize> classOf(usize glyphId) {
        auto s = begin();
        auto format = s.nextU16be();

        if (format == 1) {
            auto startGlyph = s.nextU16be();
            auto glyphCount = s.nextU16be();
            if (startGlyph <= glyphId and glyphId < startGlyph + glyphCount) {
                return s.skip((glyphId - startGlyph) * 2).nextU16be();
            }
        }

        if (format == 2) {
            auto classRangeCount = s.nextU16be();
            for (usize i : range(classRangeCount)) {
                (void)i;
                auto startGlyph = s.nextU16be();
                auto endGlyph = s.nextU16be();
                auto glyphClass = s.nextU16be();
                if (startGlyph <= glyphId and glyphId <= endGlyph) {
                    return glyphClass;
                }
            }
        }

        return NONE;
    }
};

// https://learn.microsoft.com/en-us/typography/opentype/spec/gpos#pair-adjustment-positioning-format-2-class-pair-adjustment
struct ClassPairAdjustment : public LookupSubtableBase {
    static constexpr int FORMAT = 2;

    Opt<Pair<ValueRecord>> adjustments(usize prev, usize curr) {
        auto s = begin();

        // Read the table header
        /* format = */ s.nextU16be();
        /* coverageOffset = */ s.nextU16be();
        auto valueFormat1 = s.nextU16be();
        auto valueFormat2 = s.nextU16be();
        auto classDef1Offset = s.nextU16be();
        auto classDef2Offset = s.nextU16be();
        /* class1Count = */ s.nextU16be();
        auto class2Count = s.nextU16be();

        ClassDef prevClassDef{begin().skip(classDef1Offset).restBytes()};
        ClassDef currClassDef{begin().skip(classDef2Offset).restBytes()};

        auto prevClass = try$(prevClassDef.classOf(prev));
        auto currClass = try$(currClassDef.classOf(curr));

        auto value1len = ValueRecord::len(valueFormat1);
        auto value2len = ValueRecord::len(valueFormat2);

        auto class2Size = value1len + value2len;
        auto class1Size = class2Count * class2Size;
        s.skip((prevClass * class1Size) + (currClass * class2Size));

        ValueRecord value1 = ValueRecord::read(s, valueFormat1);
        ValueRecord value2 = ValueRecord::read(s, valueFormat2);

        return Pair<ValueRecord>{value1, value2};
    }
};

using LookupSubtable = Var<
    GlyphPairAdjustment,
    ClassPairAdjustment>;

// https://learn.microsoft.com/en-us/typography/opentype/spec/chapter2#lookup-table
struct LookupTable : public BChunk {
    using LookupType = BField<u16be, 0>;
    using LookupFlag = BField<u16be, 2>;
    using SubTableCount = BField<u16be, 4>;

    enum LookupFlags : u16 {
        RIGHT_TO_LEFT = 1 << 0,
        IGNORE_BASE_GLYPHS = 1 << 1,
        IGNORE_LIGATURES = 1 << 2,
        IGNORE_MARKS = 1 << 3,
        USE_MARK_FILTERING_SET = 1 << 4,
        MARK_ATTACHMENT_TYPE = 1 << 5,
    };

    u16 lookupType() const { return get<LookupType>(); }

    u16 lookupFlag() const { return get<LookupFlag>(); }

    u16 markFilteringSet() const {
        return lookupFlag() & USE_MARK_FILTERING_SET
                   ? begin().skip(6 + get<SubTableCount>() * 2).nextU16be()
                   : 0;
    }

    LookupSubtable at(usize i) const {
        auto off = begin().skip(6 + i * 2).nextU16be();
        auto subtable = begin().skip(off);
        auto format = subtable.peekU16be();

        switch (format) {
        case GlyphPairAdjustment::FORMAT:
            return GlyphPairAdjustment{subtable.restBytes()};
        case ClassPairAdjustment::FORMAT:
            return ClassPairAdjustment{subtable.restBytes()};
        default:
            logWarn("ttf: Unknown lookup subtable format {x}", format);
            return GlyphPairAdjustment{subtable.restBytes()};
        }
    }

    usize len() const { return get<SubTableCount>(); }

    auto iter() const {
        return ::iter(*this);
    }
};

// https://learn.microsoft.com/en-us/typography/opentype/spec/chapter2#lookup-list-table
struct LookupList : public BChunk {
    using LookupCount = BField<u16be, 0>;

    usize len() const { return get<LookupCount>(); }

    LookupTable at(usize i) const {
        auto s = begin().skip(2 + i * 2);
        auto off = s.nextU16be();
        return LookupTable{begin().skip(off).restBytes()};
    }

    auto iter() const {
        return ::iter(*this);
    }
};

} // namespace Ttf
