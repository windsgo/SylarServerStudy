#pragma once

#include <string.h>
#include <assert.h>

#include "util.h"

#define SYLAR_ASSERT(x) \
    if (!(x)) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << sylar::BacktraceToString(100, 2, " -- "); \
        assert(x); \
    }

#define SYLAR_ASSERT2(x, w) \
    if (!(x)) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\ninfo: " << w \
            << "\nbacktrace:\n" \
            << sylar::BacktraceToString(100, 2, " -- "); \
        assert(x); \
    }

