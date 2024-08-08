#include <karm-text/font.h>
#include <karm-text/run.h>

#include "canvas.h"

namespace Karm::Gfx {

// MARK: Context Operations ------------------------------------------------

void Canvas::clip(Math::Recti r) {
    clip(r.cast<f64>());
}

void Canvas::clip(Math::Rectf r) {
    beginPath();
    rect(r);
    clip();
}

void Canvas::origin(Math::Vec2f p) {
    translate(p);
}

void Canvas::translate(Math::Vec2f pos) {
    transform(Math::Trans2f::makeTranslate(pos));
}

void Canvas::scale(Math::Vec2f pos) {
    transform(Math::Trans2f::makeScale(pos));
}

void Canvas::rotate(f64 angle) {
    transform(Math::Trans2f::makeRotate(angle));
}

void Canvas::skew(Math::Vec2f pos) {
    transform(Math::Trans2f::makeSkew(pos));
}

// MARK: Path Operations ---------------------------------------------------

void Canvas::fill(Fill style, FillRule rule) {
    fillStyle(style);
    fill(rule);
}

void Canvas::stroke(Stroke style) {
    strokeStyle(style);
    stroke();
}

// MARK: Shape Operations ------------------------------------------------------

void Canvas::stroke(Math::Edgef edge) {
    // dummy implementation for backends that don't support this operation
    beginPath();
    moveTo(edge.start);
    lineTo(edge.end);
    stroke();
}

void Canvas::stroke(Math::Rectf r, Math::Radiif radii) {
    // dummy implementation for backends that don't support this operation
    beginPath();
    rect(r, radii);
    stroke();
}

void Canvas::fill(Math::Rectf r, Math::Radiif radii) {
    // dummy implementation for backends that don't support this operation
    beginPath();
    rect(r, radii);
    fill();
}

void Canvas::fill(Math::Recti r, Math::Radiif radii) {
    rect(r.cast<f64>(), radii);
    fill();
}

void Canvas::stroke(Math::Ellipsef e) {
    // dummy implementation for backends that don't support this operation
    beginPath();
    ellipse(e);
    stroke();
}

void Canvas::fill(Math::Ellipsef e) {
    // dummy implementation for backends that don't support this operation
    beginPath();
    ellipse(e);
    fill();
}

void Canvas::stroke(Math::Path const &p) {
    // dummy implementation for backends that don't support this operation
    beginPath();
    path(p);
    stroke();
}

void Canvas::fill(Math::Path const &p, FillRule rule) {
    // dummy implementation for backends that don't support this operation
    beginPath();
    path(p);
    fill(rule);
}

void Canvas::fill(Text::Font &font, Text::Glyph glyph, Math::Vec2f baseline) {
    push();
    beginPath();
    origin(baseline);
    scale(font.fontsize);
    font.fontface->contour(*this, glyph);
    fill();
    pop();
}

void Canvas::fill(Text::Font &font, Text::Run const &run, Math::Vec2f baseline) {
    push();
    for (auto &cell : run._cells)
        fill(font, cell.glyph, baseline + Math::Vec2f{cell.xpos, 0});
    pop();
}

// MARK: Blit Operations ---------------------------------------------------

void Canvas::blit(Math::Recti dest, Pixels pixels) {
    blit(pixels.bound(), dest, pixels);
}

void Canvas::blit(Math::Vec2i dest, Pixels pixels) {
    blit(pixels.bound(), Math::Recti(dest, pixels.size()), pixels);
}

// MARK: Filter Operations -------------------------------------------------

void Canvas::apply(Filter filter, Math::Rectf region, Math::Radiif radii) {
    beginPath();
    rect(region, radii);
    apply(filter);
}

void Canvas::apply(Filter filter, Math::Ellipsef region) {
    beginPath();
    ellipse(region);
    apply(filter);
}

void Canvas::apply(Filter filter, Math::Path const &region) {
    beginPath();
    path(region);
    apply(filter);
}

} // namespace Karm::Gfx