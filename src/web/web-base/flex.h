#pragma once

#include "length.h"
#include "percent.h"

namespace Web {

enum struct FlexDirection {
    ROW,
    ROW_REVERSE,
    COLUMN,
    COLUMN_REVERSE,
};

enum struct FlexWrap {
    NOWRAP,
    WRAP,
    WRAP_REVERSE,
};

} // namespace Web