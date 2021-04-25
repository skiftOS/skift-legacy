#pragma once

#include <libgraphic/font/FontFace.h>
#include <libio/Reader.h>
#include <libio/ScopedReader.h>
#include <libmath/Vec2.h>
#include <libutils/unicode/Codepoint.h>

namespace Graphic::Font
{

class TrueTypeFontFace : public FontFace
{
private:
    uint16_t _num_glyphs;

    bool _has_cmap, _has_glyf, _has_loca, _has_head, _has_hhea, _has_hmtx;

    Result read_table_maxp(IO::Reader &reader);
    Result read_table_cmap(IO::Reader &reader);
    Result read_tables(IO::MemoryReader &reader);

    ResultOr<Math::Vec2<uint16_t>> read_version(IO::Reader &reader);

public:
    static ResultOr<RefPtr<TrueTypeFontFace>>
    load(IO::Reader &reader);

    virtual String family() override;
    virtual FontStyle style() override;
    virtual Optional<Glyph> glyph(Codepoint c) override;
};
} // namespace Graphic::Font