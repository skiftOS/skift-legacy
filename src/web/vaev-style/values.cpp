#include "values.h"

namespace Vaev::Style {

// MARK: Parser ----------------------------------------------------------------

// NOTE: Please keep this alphabetically sorted.

// MARK: Angle
// https://drafts.csswg.org/css-values/#angles

static Res<Angle::Unit> _parseAngleUnit(Str unit) {
    if (eqCi(unit, "deg"s))
        return Ok(Angle::Unit::DEGREE);
    else if (eqCi(unit, "grad"s))
        return Ok(Angle::Unit::GRAD);
    else if (eqCi(unit, "rad"s))
        return Ok(Angle::Unit::RADIAN);
    else if (eqCi(unit, "turn"s))
        return Ok(Angle::Unit::TURN);
    else
        return Error::invalidData("unknown length unit");
}

Res<Angle> ValueParser<Angle>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.peek() == Css::Token::DIMENSION) {
        Io::SScan scan = c->token.data;
        auto value = tryOr(Io::atof(scan), 0.0);
        auto unit = try$(_parseAngleUnit(scan.remStr()));
        return Ok(Angle{value, unit});
    }

    return Error::invalidData("expected angle");
}

// MARK: Boolean
// https://drafts.csswg.org/mediaqueries/#grid
Res<bool> ValueParser<bool>::parse(Cursor<Css::Sst> &c) {
    auto val = try$(parseValue<Integer>(c));
    return Ok(val > 0);
}

// MARK: Color
// https://drafts.csswg.org/css-color

static Res<Gfx::Color> _parseHexColor(Io::SScan &s) {
    if (s.next() != '#')
        panic("expected '#'");

    auto nextHex = [&](usize len) {
        return tryOr(Io::atou(s.slice(len), {.base = 16}), 0);
    };

    if (s.rem() == 3) {
        // #RGB
        auto r = nextHex(1);
        auto g = nextHex(1);
        auto b = nextHex(1);
        return Ok(Gfx::Color::fromRgb(r | (r << 4), g | (g << 4), b | (b << 4)));
    } else if (s.rem() == 4) {
        // #RGBA
        auto r = nextHex(1);
        auto g = nextHex(1);
        auto b = nextHex(1);
        auto a = nextHex(1);
        return Ok(Gfx::Color::fromRgba(r, g, b, a));
    } else if (s.rem() == 6) {
        // #RRGGBB
        auto r = nextHex(2);
        auto g = nextHex(2);
        auto b = nextHex(2);
        return Ok(Gfx::Color::fromRgb(r, g, b));
    } else if (s.rem() == 8) {
        // #RRGGBBAA
        auto r = nextHex(2);
        auto g = nextHex(2);
        auto b = nextHex(2);
        auto a = nextHex(2);
        return Ok(Gfx::Color::fromRgba(r, g, b, a));
    } else {
        return Error::invalidData("invalid color format");
    }
}

Res<Color> ValueParser<Color>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.peek() == Css::Token::HASH) {
        Io::SScan scan = c->token.data;
        c.next();
        return Ok(try$(_parseHexColor(scan)));
    } else if (c.peek() == Css::Token::IDENT) {
        Str data = c->token.data;
        c.next();

        auto maybeColor = parseNamedColor(data);
        if (maybeColor)
            return Ok(maybeColor.unwrap());

        auto maybeSystemColor = parseSystemColor(data);
        if (maybeSystemColor)
            return Ok(maybeSystemColor.unwrap());

        if (data == "currentColor")
            return Ok(Color::CURRENT);

        if (data == "transparent")
            return Ok(TRANSPARENT);
    }

    return Error::invalidData("expected color");
}

// MARK: Color Gamut
// https://drafts.csswg.org/mediaqueries/#color-gamut
Res<ColorGamut> ValueParser<ColorGamut>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("srgb")))
        return Ok(ColorGamut::SRGB);
    else if (c.skip(Css::Token::ident("p3")))
        return Ok(ColorGamut::DISPLAY_P3);
    else if (c.skip(Css::Token::ident("rec2020")))
        return Ok(ColorGamut::REC2020);
    else
        return Error::invalidData("expected color gamut");
}

// MARK: Display
// https://drafts.csswg.org/css-display-3/#propdef-display

static Res<Display> _parseLegacyDisplay(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("inline-block"))) {
        return Ok(Display{Display::FLOW_ROOT, Display::INLINE});
    } else if (c.skip(Css::Token::ident("inline-table"))) {
        return Ok(Display{Display::TABLE, Display::INLINE});
    } else if (c.skip(Css::Token::ident("inline-flex"))) {
        return Ok(Display{Display::FLEX, Display::INLINE});
    } else if (c.skip(Css::Token::ident("inline-grid"))) {
        return Ok(Display{Display::GRID, Display::INLINE});
    }

    return Error::invalidData("expected legacy display value");
}

static Res<Display::Outside> _parseOutsideDisplay(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("block"))) {
        return Ok(Display::BLOCK);
    } else if (c.skip(Css::Token::ident("inline"))) {
        return Ok(Display::INLINE);
    } else if (c.skip(Css::Token::ident("run-in"))) {
        return Ok(Display::RUN_IN);
    }

    return Error::invalidData("expected outside value");
}

static Res<Display::Inside> _parseInsideDisplay(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("flow"))) {
        return Ok(Display::FLOW);
    } else if (c.skip(Css::Token::ident("flow-root"))) {
        return Ok(Display::FLOW_ROOT);
    } else if (c.skip(Css::Token::ident("flex"))) {
        return Ok(Display::FLEX);
    } else if (c.skip(Css::Token::ident("grid"))) {
        return Ok(Display::GRID);
    } else if (c.skip(Css::Token::ident("ruby"))) {
        return Ok(Display::RUBY);
    } else if (c.skip(Css::Token::ident("table"))) {
        return Ok(Display::TABLE);
    } else if (c.skip(Css::Token::ident("math"))) {
        return Ok(Display::MATH);
    }

    return Error::invalidData("expected inside value");
}

static Res<Display> _parseInternalDisplay(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("table-row-group"))) {
        return Ok(Display::TABLE_ROW_GROUP);
    } else if (c.skip(Css::Token::ident("table-header-group"))) {
        return Ok(Display::TABLE_HEADER_GROUP);
    } else if (c.skip(Css::Token::ident("table-footer-group"))) {
        return Ok(Display::TABLE_FOOTER_GROUP);
    } else if (c.skip(Css::Token::ident("table-row"))) {
        return Ok(Display::TABLE_ROW);
    } else if (c.skip(Css::Token::ident("table-cell"))) {
        return Ok(Display::TABLE_CELL);
    } else if (c.skip(Css::Token::ident("table-column-group"))) {
        return Ok(Display::TABLE_COLUMN_GROUP);
    } else if (c.skip(Css::Token::ident("table-column"))) {
        return Ok(Display::TABLE_COLUMN);
    } else if (c.skip(Css::Token::ident("table-caption"))) {
        return Ok(Display::TABLE_CAPTION);
    } else if (c.skip(Css::Token::ident("ruby-base"))) {
        return Ok(Display::RUBY_BASE);
    } else if (c.skip(Css::Token::ident("ruby-text"))) {
        return Ok(Display::RUBY_TEXT);
    } else if (c.skip(Css::Token::ident("ruby-base-container"))) {
        return Ok(Display::RUBY_BASE_CONTAINER);
    } else if (c.skip(Css::Token::ident("ruby-text-container"))) {
        return Ok(Display::RUBY_TEXT_CONTAINER);
    }

    return Error::invalidData("expected internal value");
}

static Res<Display> _parseBoxDisplay(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("contents"))) {
        return Ok(Display::CONTENTS);
    } else if (c.skip(Css::Token::ident("none"))) {
        return Ok(Display::NONE);
    }

    return Error::invalidData("expected box value");
}

Res<Display> ValueParser<Display>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (auto legacy = _parseLegacyDisplay(c))
        return legacy;

    if (auto box = _parseBoxDisplay(c))
        return box;

    if (auto internal = _parseInternalDisplay(c))
        return internal;

    auto inside = _parseInsideDisplay(c);
    auto outside = _parseOutsideDisplay(c);
    auto item = c.skip(Css::Token::ident("list-item"))
                    ? Display::Item::YES
                    : Display::Item::NO;

    if (not inside and not outside and not bool(item))
        return Error::invalidData("expected display value");

    return Ok(Display{
        tryOr(inside, Display::FLOW),
        tryOr(outside, Display::BLOCK),
        item,
    });
}

// MARK: FlexDirection
// https://drafts.csswg.org/css-flexbox-1/#flex-direction-property
Res<FlexDirection> ValueParser<FlexDirection>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("row")))
        return Ok(FlexDirection::ROW);
    else if (c.skip(Css::Token::ident("row-reverse")))
        return Ok(FlexDirection::ROW_REVERSE);
    else if (c.skip(Css::Token::ident("column")))
        return Ok(FlexDirection::COLUMN);
    else if (c.skip(Css::Token::ident("column-reverse")))
        return Ok(FlexDirection::COLUMN_REVERSE);
    else
        return Error::invalidData("expected flex direction");
}

// Mark: FlexWrap
// https://drafts.csswg.org/css-flexbox-1/#flex-wrap-property
Res<FlexWrap> ValueParser<FlexWrap>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("nowrap")))
        return Ok(FlexWrap::NOWRAP);
    else if (c.skip(Css::Token::ident("wrap")))
        return Ok(FlexWrap::WRAP);
    else if (c.skip(Css::Token::ident("wrap-reverse")))
        return Ok(FlexWrap::WRAP_REVERSE);
    else
        return Error::invalidData("expected flex wrap");
}

// MARK: FlexBasis
// https://drafts.csswg.org/css-flexbox-1/#flex-basis-property
Res<FlexBasis> ValueParser<FlexBasis>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("content")))
        return Ok(FlexBasis{FlexBasis::CONTENT});

    return Ok(try$(parseValue<Width>(c)));
}

// MARK: FontSize
// https://www.w3.org/TR/css-fonts-4/#font-size-prop

Res<FontSize> ValueParser<FontSize>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("xx-small")))
        return Ok(FontSize::XX_SMALL);
    else if (c.skip(Css::Token::ident("x-small")))
        return Ok(FontSize::X_SMALL);
    else if (c.skip(Css::Token::ident("small")))
        return Ok(FontSize::SMALL);
    else if (c.skip(Css::Token::ident("medium")))
        return Ok(FontSize::MEDIUM);
    else if (c.skip(Css::Token::ident("large")))
        return Ok(FontSize::LARGE);
    else if (c.skip(Css::Token::ident("x-large")))
        return Ok(FontSize::X_LARGE);
    else if (c.skip(Css::Token::ident("xx-large")))
        return Ok(FontSize::XX_LARGE);
    else if (c.skip(Css::Token::ident("smaller")))
        return Ok(FontSize::SMALLER);
    else if (c.skip(Css::Token::ident("larger")))
        return Ok(FontSize::LARGER);
    else if (c.skip(Css::Token::ident("math")))
        return Ok(FontSize::MATH);
    else
        return Ok(try$(parseValue<PercentOr<Length>>(c)));
}

// MARK: FontStyle
// https://drafts.csswg.org/css-fonts-4/#propdef-font-style

Res<FontStyle> ValueParser<FontStyle>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("normal"))) {
        return Ok(FontStyle::NORMAL);
    } else if (c.skip(Css::Token::ident("italic"))) {
        return Ok(FontStyle::ITALIC);
    } else if (c.skip(Css::Token::ident("oblique"))) {
        auto angle = parseValue<Angle>(c);
        if (angle)
            return Ok(angle.unwrap());

        return Ok(FontStyle::OBLIQUE);
    }

    return Error::invalidData("expected font style");
}

// MARK: FontWeight
// https://www.w3.org/TR/css-fonts-4/#font-weight-absolute-values

Res<FontWeight> ValueParser<FontWeight>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("normal"))) {
        return Ok(FontWeight::NORMAL);
    } else if (c.skip(Css::Token::ident("bold"))) {
        return Ok(FontWeight::BOLD);
    } else if (c.skip(Css::Token::ident("bolder"))) {
        return Ok(FontWeight::BOLDER);
    } else if (c.skip(Css::Token::ident("lighter"))) {
        return Ok(FontWeight::LIGHTER);
    } else {
        return Ok(try$(parseValue<Integer>(c)));
    }
}

// MARK: FontWidth
// https://www.w3.org/TR/css-fonts-4/#propdef-font-width
Res<FontWidth> ValueParser<FontWidth>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("normal"))) {
        return Ok(FontWidth::NORMAL);
    } else if (c.skip(Css::Token::ident("condensed"))) {
        return Ok(FontWidth::CONDENSED);
    } else if (c.skip(Css::Token::ident("expanded"))) {
        return Ok(FontWidth::EXPANDED);
    } else if (c.skip(Css::Token::ident("ultra-condensed"))) {
        return Ok(FontWidth::ULTRA_CONDENSED);
    } else if (c.skip(Css::Token::ident("extra-condensed"))) {
        return Ok(FontWidth::EXTRA_CONDENSED);
    } else if (c.skip(Css::Token::ident("semi-condensed"))) {
        return Ok(FontWidth::SEMI_CONDENSED);
    } else if (c.skip(Css::Token::ident("semi-expanded"))) {
        return Ok(FontWidth::SEMI_EXPANDED);
    } else if (c.skip(Css::Token::ident("extra-expanded"))) {
        return Ok(FontWidth::EXTRA_EXPANDED);
    } else if (c.skip(Css::Token::ident("ultra-expanded"))) {
        return Ok(FontWidth::ULTRA_EXPANDED);
    }

    return Ok(try$(parseValue<Percent>(c)));
}

// MARK: Hover
// https://drafts.csswg.org/mediaqueries/#hover
Res<Hover> ValueParser<Hover>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("none")))
        return Ok(Hover::NONE);
    else if (c.skip(Css::Token::ident("hover")))
        return Ok(Hover::HOVER);
    else
        return Error::invalidData("expected hover value");
}

// MARK: Integer
// https://drafts.csswg.org/css-values/#integers
Res<Integer> ValueParser<Integer>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.peek() == Css::Token::NUMBER) {
        Io::SScan scan = c->token.data;
        return Ok(try$(Io::atoi(scan)));
    }

    return Error::invalidData("expected integer");
}

// MARK: Length
// https://drafts.csswg.org/css-values/#lengths

static Res<Length::Unit> _parseLengthUnit(Str unit) {
#define LENGTH(NAME, ...)      \
    if (eqCi(unit, #NAME ""s)) \
        return Ok(Length::Unit::NAME);
#include <vaev-base/defs/lengths.inc>
#undef LENGTH

    return Error::invalidData("unknown length unit");
}

Res<Length> ValueParser<Length>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.peek() == Css::Token::DIMENSION) {
        Io::SScan scan = c->token.data;
        auto value = tryOr(Io::atof(scan), 0.0);
        auto unit = try$(_parseLengthUnit(scan.remStr()));
        return Ok(Length{value, unit});
    } else if (c.peek() == Css::Token::number("0")) {
        return Ok(Length{0.0, Length::Unit::PX});
    }

    return Error::invalidData("expected length");
}

// MARL: MarginWidth
// https://drafts.csswg.org/css-values/#margin-width

Res<Width> ValueParser<Width>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c->token == Css::Token::ident("auto")) {
        c.next();
        return Ok(Width::AUTO);
    }

    return Ok(try$(parseValue<PercentOr<Length>>(c)));
}

// MARK: MediaType
// https://drafts.csswg.org/mediaqueries/#media-types

Res<MediaType> ValueParser<MediaType>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("all")))
        return Ok(MediaType::ALL);

    if (c.skip(Css::Token::ident("print")))
        return Ok(MediaType::PRINT);

    if (c.skip(Css::Token::ident("screen")))
        return Ok(MediaType::SCREEN);

    // NOTE: The spec says that we should reconize the following media types
    //       but they should not match anything
    static constexpr Array OTHER = {
        "tty",
        "tv",
        "projection",
        "handheld",
        "braille",
        "embossed",
        "speech",
        "aural",
    };

    for (auto const &item : OTHER) {
        if (c.skip(Css::Token::ident(item)))
            return Ok(MediaType::OTHER);
    }
    return Error::invalidData("expected media type");
}

// MARK: Number
// https://drafts.csswg.org/css-values/#numbers

Res<Number> ValueParser<Number>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.peek() == Css::Token::NUMBER) {
        Io::SScan scan = c->token.data;
        return Ok(try$(Io::atof(scan)));
    }

    return Error::invalidData("expected number");
}

// MARK: Orientation
// https://drafts.csswg.org/mediaqueries/#orientation
Res<Orientation> ValueParser<Orientation>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("portrait")))
        return Ok(Orientation::PORTRAIT);
    else if (c.skip(Css::Token::ident("landscape")))
        return Ok(Orientation::LANDSCAPE);
    else
        return Error::invalidData("expected orientation");
}

// MARK: Overflow
// https://www.w3.org/TR/css-overflow/#overflow-control
Res<Overflow> ValueParser<Overflow>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("overlay")))
        // https://www.w3.org/TR/css-overflow/#valdef-overflow-overlay
        return Ok(Overflow::AUTO);
    else if (c.skip(Css::Token::ident("visible")))
        return Ok(Overflow::VISIBLE);
    else if (c.skip(Css::Token::ident("hidden")))
        return Ok(Overflow::HIDDEN);
    else if (c.skip(Css::Token::ident("scroll")))
        return Ok(Overflow::SCROLL);
    else if (c.skip(Css::Token::ident("auto")))
        return Ok(Overflow::AUTO);
    else
        return Error::invalidData("expected overflow value");
}

// MARK: OverflowBlock
// https://drafts.csswg.org/mediaqueries/#mf-overflow-block
Res<OverflowBlock> ValueParser<OverflowBlock>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("none")))
        return Ok(OverflowBlock::NONE);
    else if (c.skip(Css::Token::ident("scroll")))
        return Ok(OverflowBlock::SCROLL);
    else if (c.skip(Css::Token::ident("paged")))
        return Ok(OverflowBlock::PAGED);
    else
        return Error::invalidData("expected overflow block value");
}

// MARK: OverflowInline
// https://drafts.csswg.org/mediaqueries/#mf-overflow-inline
Res<OverflowInline> ValueParser<OverflowInline>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("none")))
        return Ok(OverflowInline::NONE);
    else if (c.skip(Css::Token::ident("scroll")))
        return Ok(OverflowInline::SCROLL);
    else
        return Error::invalidData("expected overflow inline value");
}

// MARK: Percentage
// https://drafts.csswg.org/css-values/#percentages

Res<Percent> ValueParser<Percent>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.peek() == Css::Token::PERCENTAGE) {
        Io::SScan scan = c->token.data;
        auto value = tryOr(Io::atof(scan), 0.0);
        if (scan.remStr() != "%")
            return Error::invalidData("invalid percentage");

        return Ok(Percent{value});
    }

    return Error::invalidData("expected percentage");
}

// MARK: Pointer
// https://drafts.csswg.org/mediaqueries/#pointer
Res<Pointer> ValueParser<Pointer>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("none")))
        return Ok(Pointer::NONE);
    else if (c.skip(Css::Token::ident("coarse")))
        return Ok(Pointer::COARSE);
    else if (c.skip(Css::Token::ident("fine")))
        return Ok(Pointer::FINE);
    else
        return Error::invalidData("expected pointer value");
}

// Mark: Resolution
// https://drafts.csswg.org/css-values/#resolution

static Res<Resolution::Unit> _parseResolutionUnit(Str unit) {
    if (eqCi(unit, "dpi"s))
        return Ok(Resolution::Unit::DPI);
    else if (eqCi(unit, "dpcm"s))
        return Ok(Resolution::Unit::DPCM);
    else if (eqCi(unit, "dppx"s))
        return Ok(Resolution::Unit::DPPX);
    else
        return Error::invalidData("unknown resolution unit");
}

Res<Resolution> ValueParser<Resolution>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.peek() == Css::Token::DIMENSION) {
        Io::SScan scan = c->token.data;
        auto value = tryOr(Io::atof(scan), 0.0);
        auto unit = try$(_parseResolutionUnit(scan.remStr()));
        return Ok(Resolution{value, unit});
    }

    return Error::invalidData("expected resolution");
}

// MARK: Scan
// https://drafts.csswg.org/mediaqueries/#scan
Res<Scan> ValueParser<Scan>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("interlace")))
        return Ok(Scan::INTERLACE);
    else if (c.skip(Css::Token::ident("progressive")))
        return Ok(Scan::PROGRESSIVE);
    else
        return Error::invalidData("expected scan value");
}

// MARK: Size
// https://drafts.csswg.org/css-sizing-4/#sizing-values

Res<Size> ValueParser<Size>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.peek() == Css::Token::IDENT) {
        auto data = c.next().token.data;
        if (data == "auto") {
            return Ok(Size::AUTO);
        } else if (data == "none") {
            return Ok(Size::NONE);
        } else if (data == "min-content") {
            return Ok(Size::MIN_CONTENT);
        } else if (data == "max-content") {
            return Ok(Size::MAX_CONTENT);
        }
    } else if (c.peek() == Css::Token::PERCENTAGE) {
        return Ok(try$(parseValue<Percent>(c)));
    } else if (c.peek() == Css::Token::DIMENSION) {
        return Ok(try$(parseValue<Length>(c)));
    } else if (c.peek() == Css::Sst::FUNC) {
        auto const &prefix = c.next().prefix.unwrap();
        auto prefixToken = prefix->token;
        if (prefixToken.data == "fit-content") {
            Cursor<Css::Sst> content = prefix->content;
            return Ok(Size{Size::FIT_CONTENT, try$(parseValue<Length>(content))});
        }
    }

    return Error::invalidData("expected size");
}

// MARK: String
// https://drafts.csswg.org/css-values/#strings

Res<String> ValueParser<String>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.peek() == Css::Token::STRING) {
        // TODO: Handle escape sequences
        Io::SScan s = c.next().token.data;
        StringBuilder sb{s.rem()};
        auto quote = s.next();
        while (not s.skip(quote) and not s.ended()) {
            if (s.skip('\\') and not s.ended()) {
                if (s.skip('\\'))
                    sb.append(s.next());
            } else {
                sb.append(s.next());
            }
        }
        return Ok(sb.take());
    }

    return Error::invalidData("expected string");
}

// MARK: Update
// https://drafts.csswg.org/mediaqueries/#update
Res<Update> ValueParser<Update>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("none")))
        return Ok(Update::NONE);
    else if (c.skip(Css::Token::ident("slow")))
        return Ok(Update::SLOW);
    else if (c.skip(Css::Token::ident("fast")))
        return Ok(Update::FAST);
    else
        return Error::invalidData("expected update value");
}

// MARK: ReducedMotion
// https://drafts.csswg.org/mediaqueries/#reduced-motion
Res<ReducedMotion> ValueParser<ReducedMotion>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("no-preference")))
        return Ok(ReducedMotion::NO_PREFERENCE);
    else if (c.skip(Css::Token::ident("reduce")))
        return Ok(ReducedMotion::REDUCE);
    else
        return Error::invalidData("expected reduced motion value");
}

// MARK: ReducedTransparency
// https://drafts.csswg.org/mediaqueries/#reduced-transparency
Res<ReducedTransparency> ValueParser<ReducedTransparency>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("no-preference")))
        return Ok(ReducedTransparency::NO_PREFERENCE);
    else if (c.skip(Css::Token::ident("reduce")))
        return Ok(ReducedTransparency::REDUCE);
    else
        return Error::invalidData("expected reduced transparency value");
}

// MARK: Contrast
// https://drafts.csswg.org/mediaqueries/#contrast
Res<Contrast> ValueParser<Contrast>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("less")))
        return Ok(Contrast::LESS);
    else if (c.skip(Css::Token::ident("more")))
        return Ok(Contrast::MORE);
    else
        return Error::invalidData("expected contrast value");
}

// MARK: Colors
// https://drafts.csswg.org/mediaqueries/#forced-colors
Res<Colors> ValueParser<Colors>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("none")))
        return Ok(Colors::NONE);
    else if (c.skip(Css::Token::ident("active")))
        return Ok(Colors::ACTIVE);
    else
        return Error::invalidData("expected colors value");
}

// MARK: ColorScheme
// https://drafts.csswg.org/mediaqueries/#color-scheme
Res<ColorScheme> ValueParser<ColorScheme>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("light")))
        return Ok(ColorScheme::LIGHT);
    else if (c.skip(Css::Token::ident("dark")))
        return Ok(ColorScheme::DARK);
    else
        return Error::invalidData("expected color scheme value");
}

// MARK: ReducedData
// https://drafts.csswg.org/mediaqueries/#reduced-data
Res<ReducedData> ValueParser<ReducedData>::parse(Cursor<Css::Sst> &c) {
    if (c.ended())
        return Error::invalidData("unexpected end of input");

    if (c.skip(Css::Token::ident("no-preference")))
        return Ok(ReducedData::NO_PREFERENCE);
    else if (c.skip(Css::Token::ident("reduce")))
        return Ok(ReducedData::REDUCE);
    else
        return Error::invalidData("expected reduced data value");
}

} // namespace Vaev::Style
