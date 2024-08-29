
#include <karm-ui/dialog.h>
#include <karm-ui/layout.h>
#include <karm-ui/scroll.h>

#include "about-dialog.h"
#include "badge.h"
#include "dialog.h"

namespace Karm::Kira {

static constexpr Str LICENSE = R"(Copyright © 2018-2024, the skiftOS Developers

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.)";

Ui::Child licenseDialog() {
    return dialogContent({
        dialogTitleBar("License"s),
        Ui::bodySmall(LICENSE) |
            Ui::insets(16) |
            Ui::vscroll() |
            Ui::maxSize({480, Ui::UNCONSTRAINED}) |
            Ui::grow(),
    });
}

Ui::Child aboutDialog(String name) {
    return dialogContent({
        dialogHeader({
            Ui::hflow(6, Math::Align::CENTER, dialogTitle(name), versionBadge()),
            dialogDescription("Copyright © 2018-2024 The skiftOS Developers\n"
                              "\n"
                              "All rights reserved."s),
        }),
        dialogFooter({
            button([](auto &n) {
                showDialog(n, licenseDialog());
            },
                   Ui::ButtonStyle::regular(), "License"),
            dialogAction(Ui::NOP, "Ok"s),
        }),
    });
}

} // namespace Karm::Kira
