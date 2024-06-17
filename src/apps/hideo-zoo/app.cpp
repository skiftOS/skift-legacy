#include <hideo-base/scafold.h>

#include "app.h"
#include "model.h"

// Pages
#include "page-avatar.h"
#include "page-badge.h"
#include "page-checkbox.h"
#include "page-context-menu.h"
#include "page-radio.h"
#include "page-sidenav.h"
#include "page-slider.h"
#include "page-toggle.h"

namespace Hideo::Zoo {

static Array PAGES = {
    &PAGE_AVATAR,
    &PAGE_BADGE,
    &PAGE_CHECKBOX,
    &PAGE_CONTEXT_MENU,
    &PAGE_RADIO,
    &PAGE_SIDENAV,
    &PAGE_SLIDER,
    &PAGE_TOGGLE,
};

Ui::Child app() {
    return Ui::reducer<Model>([](State const &s) {
        return scafold({
            .icon = Mdi::DUCK,
            .title = "Zoo"s,
            .sidebar = [&] {
                return Kr::sidenav(
                    iter(PAGES)
                        .mapi([&](Page const *page, usize index) {
                            return Kr::sidenavItem(
                                index == s.page,
                                Model::bind<Switch>(index),
                                page->icon,
                                page->name
                            );
                        })
                        .collect<Ui::Children>()
                );
            },
            .body = [&] {
                auto &page = PAGES[s.page];
                return Ui::vflow(
                    Ui::vflow(
                        Ui::titleLarge(page->name),
                        Ui::empty(4),
                        Ui::bodyMedium(page->description)
                    ) | Ui::spacing(16),
                    Ui::separator(),
                    page->build() | Ui::grow()
                );
            },
        });
    });
}

} // namespace Hideo::Zoo