#pragma once

#include <cstddef>
#include <limits>

namespace ds
{
    // vrati maximalnu hodnotu daneho ciselneho typu => budeme ju pouzivat ako neplatny index
    const size_t INVALID_INDEX = (std::numeric_limits<size_t>::max)();
}