
#pragma once

#include <cstdint>

using int32 = int32_t;
using int16 = int16_t;
using int8 = int8_t;

namespace TS {
struct file_signal
{
    int32 dzi : 12;
    int32 ns : 8;
    int32 vvi : 12;
};
struct signal
{
    int32 kns;
    int32 dzi;
    int32 vvi;
};
}
